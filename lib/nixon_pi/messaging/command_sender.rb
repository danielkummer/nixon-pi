module NixonPi
  module Messaging
    class CommandSender
      include BunnyConnection

      ##
      # Send an async command to a registered commandreceiver
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

      def send_request(target, payload)
        log.info "publishing request #{payload} to #{target}"
        topic(:request).publish(payload.to_json,
                                routing_key: 'nixonpi.request',
                                content_type: 'application/json',
                                type: target,
                                immediate: true
                               )
      end

      def send_response(target, payload)
        log.info "publishing response #{payload} to #{target}"
        topic(:response).publish(payload.to_json,
                                 routing_key: 'nixonpi.response',
                                 content_type: 'application/json',
                                 type: target,
                                 immediate: true)
      end
    end
  end
end
