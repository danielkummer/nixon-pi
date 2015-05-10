require_relative 'client'
require_relative '../logging/logging'

module NixonPi
  module Messaging
    class CommandSender
      include Client

      ##
      # Send an async command to a registered commandreceiver
      # @param [Symbol] target
      # @param [Hash] payload
      def send_command(target, payload)
        log.info "publishing #{payload} to #{target}"
        direct_exchange_channel.publish(payload.to_json,
                                        routing_key: 'nixonpi.command',
                                        content_type: 'application/json',
                                        type: target,
                                        immediate: true)
      end
    end
  end
end
