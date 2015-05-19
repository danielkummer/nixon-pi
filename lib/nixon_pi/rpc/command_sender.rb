module NixonPi
  module RPC
    class CommandSender
      include BunnyConnection

      ##
      # RPC! Send an async command to a registered commandreceiver
      #
      # @param [Symbol] target
      # @param [Hash] payload
      def send_command(target, payload)
        log.info "publishing #{payload} to #{target}"
        exchange(:command)
            .publish(payload.to_json,
                     routing_key: 'nixonpi.command.general',
                     content_type: 'application/json',
                     type: target,
                     immediate: true)
      end


    end
  end
end
