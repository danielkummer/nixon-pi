module NixonPi
  module RabbitMQ
    class CommandSender
      include BunnyConnection

      ##
      # Send an async command to a registered commandreceiver
      #
      # @param [Symbol] target
      # @param [Hash] payload
      def send_command(target, payload)
        log.info "publishing #{payload} to #{target}"
        topic(:command).publish(payload.to_json,
                                routing_key: 'nixonpi.command',
                                content_type: 'application/json',
                                type: target,
                                immediate: true)
      end

      ##
      # Send an async request to a registered command receiver
      #
      # @param [Symbol] target
      # @param [Hash] payload
      def send_request(target, payload)
        log.info "publishing request #{payload} to #{target}"
        topic(:request).publish(payload.to_json,
                                routing_key: 'nixonpi.request',
                                content_type: 'application/json',
                                type: target,
                                reply_to: 'nixonpi.reply',
                                immediate: true)
      end
    end
  end
end
