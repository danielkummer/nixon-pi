require 'bunny'
require 'json'

module NixonPi
  module RabbitMQ
    ##
    # Proxy for information requests
    class InformationInbox
      include Logging
      include BunnyConnection

      def initialize

        @receivers = ThreadSafe::Cache.new

        @queue = channel.queue('information')
        @inbox = @queue.bind(topic(:request), routing_key: 'nixonpi.command')
                     .subscribe do |_delivery_info, metadata, data|
          if metadata[:content_type] == 'application/json'
            begin
              data_hash = JSON.parse(data, {symbolize_names: true})
              log.info "Handling info request #{data} with metadata: #{metadata.to_s}"
              handle_info_request_for(metadata.type, data_hash)

              topic(:response).publish(data.to_json,
                                       routing_key: metadata.reply_to,
                                       correlation_id: metadata.correlation_id,
                                       content_type: 'application/json',
                                       type: target,
                                       immediate: true)

            rescue JSON::ParserError => e
              log.error("Error parsing json: #{e.message}")
            end
          else
            log.error("No handler for content type: #{metadata[:content_type]}")
          end
        end
      end

      ##
      # Add object which uses the InfoResponder module
      # @param [InfoResponder] receiver_instance
      # @param [Symbol] target name under which the receiver listens for information requests
      def add_target(receiver_instance, target)
        target = target.to_sym
        if @receivers[target].nil?
          @receivers[target] = Array.wrap(receiver_instance)
        else
          @receivers[target] << receiver_instance
        end
      end

      ##
      # Get information from a specified target
      # @param [Symbol] target information from whom
      # @param [Symbol] about information about what
      def handle_info_request_for(target, about)
        target = target.to_sym
        returned_info = []

        @receivers[target].each do |receiver|
          if receiver.respond_to?(:handle_info_request) then
            returned_info << receiver.handle_info_request(about)
          else
            log.error "Listener for #{target} doesn't have the receive method!"
          end
        end
        returned_info = returned_info[0] if returned_info.size == 1 # array to single if only one element
        returned_info
      end
    end
  end
end

