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
        channel.topic('nixonpi.rpc.command').publish(payload.to_json,
                                routing_key: 'nixonpi.command',
                                content_type: 'application/json',
                                type: target,
                                immediate: true)
      end


    end
  end
end
