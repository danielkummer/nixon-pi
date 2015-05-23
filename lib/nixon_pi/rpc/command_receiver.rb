require 'json'
require 'bunny'

module NixonPi
  module RPC
    class CommandReceiver
      include BunnyConnection

      # catch all commands
      ROUTING_KEY = 'nixonpi.command.#'

      ##
      # Constructor
      # @raise [Bunny::TCPConnectionFailed] if rabbitmq server not reachable
      def initialize
        @receivers = ThreadSafe::Cache.new
        channel
            .queue(named_queue(:command), exclusive: true)
            .bind(exchange(:command), routing_key: ROUTING_KEY)
            .subscribe { |_delivery_info, metadata, data| received_command(_delivery_info, metadata, data) }
      end

      ##
      # Add a receiver
      #
      # @param [Hash] command command to listen to
      # @param [Commands] receiver
      def add_receiver(receiver, command)
        command = command.to_sym
        fail "Receiver #{receiver.class} must include the receiver module" unless receiver.is_a?(Commands)
        @receivers[command] ||= []
        @receivers[command] << receiver
      end

      ##
      # RPC!
      # Get information about the command receiver
      # @param [Symbol] about information identifier
      # @return [Hash] information hash
      def handle_information_request(about)
        case about.to_sym
          when :receivers
            {receivers: @receivers.keys}
          else
            log.error "no information about '#{about}' found"
            {}
        end
      end

      protected

      ##
      # Handle a received command
      # @param [Hash] delivery_info delivery information
      # @param [Hash] metadata delivery metadata
      # @param [JSON] data delivery payload
      def received_command(delivery_info, metadata, data)
        if metadata[:content_type] == 'application/json'
          begin
            delegate_command(metadata.type, JSON.parse(data, {symbolize_names: true}))
          rescue JSON::ParserError => e
            log.error("Error parsing JSON: #{e.message}")
          end
        else
          log.error("No handler for content type: #{metadata[:content_type]}")
        end
      end

      ##
      # Internal command handler
      # @param [Symbol] target the command target - must be the same name used in add_receiver
      # @param [Hash] data
      def delegate_command(target, data)
        target = target.to_sym
        log.debug "Handle command for #{target} with: #{data}"
        @receivers[target].each do |receiver|
          fail "#{receiver} doesn't have the handle_command method!" unless receiver.respond_to?(:handle_command)
          # TODO: validate data here!! + what about scheduler?
          model_data = data.clone
          model_data[:target] = target
          model_data.delete(:id)
          # TODO: locking here isn't good
          validator_model = target == :schedule ? Schedule.new(model_data) : Command.new(model_data)
          if validator_model.valid?
            data.delete_if { |k, v| !receiver.class.available_commands.include?(k.to_sym) || v.nil? }
            receiver.handle_command(data)
          else
            log.error "Invalid command received: #{data}, errors: #{validator_model.errors.full_messages.join("\n")}"
          end

        end if @receivers[target]
      end
    end
  end
end