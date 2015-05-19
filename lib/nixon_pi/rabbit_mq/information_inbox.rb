require 'bunny'
require 'json'

module NixonPi
  module RabbitMQ
    ##
    # Proxy for information requests
    # Consumer
    class InformationInbox
      include Logging
      include BunnyConnection

      def initialize
        @receivers = ThreadSafe::Cache.new

        exchange = channel.topic('nixonpi.rpc.reqest')

        @inbox = channel
                     .queue('nixonpi.rpc.request')
                     .bind(exchange).subscribe do |_delivery_info, metadata, data|
          log.debug "Handling info request #{data} with metadata: #{metadata.to_s}"
          if metadata[:content_type] == 'application/json'
            begin
              request_data = JSON.parse(data, {symbolize_names: true})

              #todo create custom error and log correctly...
              raise 'Type not set in metadata!' if metadata.type.nil?

              #todo this is a work-around because the handling method only accepts symbols, not a hash
              response = handle_info_request_for(metadata.type, request_data[:about])

              log.error "Sending reply: #{metadata}, with response #{response}"

              reply_exchange = channel.topic('nixonpi.rpc.request')

              channel
                  .queue('nixonpi.rpc.reply')
                  .bind(reply_exchange)
                  .publish(response.to_json,
                           routing_key: metadata.reply_to,
                           correlation_id: metadata.correlation_id,
                           content_type: 'application/json',
                           type: request_data[:target],
                           immediate: true)

            rescue JSON::ParserError => e
              log.error("Error parsing json: #{e.message}")
              #rescue Exception => e
              #  log.error "Error while processing information request: #{e.message}"
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

        #special case, if target is an enummerated instance, add to a grouped target...
        enumerated = /([a-zA-Z]+)\d+$/.match(target)
        if enumerated
          basename = enumerated[1]
          targetcollection = "#{basename}s"
          @receivers[targetcollection.to_sym] ||= []
          @receivers[targetcollection.to_sym] << receiver_instance
        end
        @receivers[target.to_sym] ||= []
        @receivers[target.to_sym] << receiver_instance
      end

      ##
      # Get information from a specified target
      # @param [Symbol] target information from whom
      # @param [Symbol] about information about what
      def handle_info_request_for(target, about)
        target = target.to_sym
        returned_info = []
        @receivers[target].each do |receiver|
          if receiver.respond_to?(:handle_info_request)
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

