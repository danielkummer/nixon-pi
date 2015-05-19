require 'bunny'
require 'json'

module NixonPi
  module RPC
    ##
    # Proxy for information requests
    # Consumer
    class InformationRequestReceiver
      include Logging
      include BunnyConnection

      ROUTING_KEY = 'nixonpi.rpc.request'

      def initialize
        @receivers = ThreadSafe::Cache.new
        channel
            .queue(ROUTING_KEY)
            .bind(exchange(:request))
            .subscribe { |_delivery_info, metadata, data| received_information_request(_delivery_info, metadata, data) }
      end

      ##
      # Add object which uses the InformationResponder module
      # @param [Object] receiver_instance
      # @param [Symbol] target name under which the receiver listens for information requests
      def add_target(receiver_instance, target)
        #special case, if target is an enummerated instance, add to a grouped target...
        if (is_enumerated = /([a-zA-Z]+)\d+$/.match(target))
          target_collection = "#{is_enumerated[1]}s" # transform lamp0 to lamps... (for example)
          @receivers[target_collection.to_sym] ||= []
          @receivers[target_collection.to_sym] << receiver_instance
        end
        @receivers[target.to_sym] ||= []
        @receivers[target.to_sym] << receiver_instance
      end

      protected
      def received_information_request(delivery_info, metadata, payload)
        fail 'Type not set in metadata!' if metadata.type.nil?
        log.debug "Handling info request #{payload} with metadata: #{metadata.to_s}"
        if metadata[:content_type] == 'application/json'
          begin
            request_data = JSON.parse(payload, {symbolize_names: true})

            #TODO : this is a work-around because the handling method only accepts symbols, not a hash
            response = delegate_information_request(metadata.type, request_data[:about])

            # Publish response
            channel
                .queue(named_queue(:reply))
                .bind(exchange(:reply))
                .publish(response.to_json,
                         routing_key: metadata.reply_to,
                         correlation_id: metadata.correlation_id,
                         content_type: 'application/json',
                         type: request_data[:target],
                         immediate: true)

          rescue JSON::ParserError => e
            log.error("Error parsing JSON: #{e.message}")
          end
        else
          log.error("No handler for content type: #{metadata[:content_type]}")
        end
      end

      ##
      # Get information from a specified target
      # @param [Symbol] target information from whom
      # @param [Symbol] about information about what
      # @return [Array] return all requested information, if only one object is present, return the one objectxw
      def delegate_information_request(target, about)
        target = target.to_sym
        returned_info = []
        @receivers[target].each do |receiver|
          fail "#{target} doesn't have the receive method!" unless receiver.respond_to?(:handle_information_request)
          returned_info << receiver.handle_information_request(about)
        end
        returned_info = returned_info[0] if returned_info.size == 1 # array to single if only one element
        returned_info
      end
    end
  end
end

