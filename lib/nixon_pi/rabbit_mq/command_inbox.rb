require 'json'
require 'singleton'
require 'bunny'

module NixonPi
  module RabbitMQ
    class CommandInbox
      include BunnyConnection
      include InfoResponder

      ##
      # Constructor
      # @raise [Bunny::TCPConnectionFailed] if rabbitmq server not reachable
      def initialize
        @receivers = ThreadSafe::Cache.new

        exchange = channel.topic('nixonpi.rpc.command')

        @inbox = channel
                     .queue('nixonpi.rpc.command', exclusive: true)
                     .bind(exchange, routing_key: 'nixonpi.command.#')
                     .subscribe do |_delivery_info, metadata, data|
          if metadata[:content_type] == 'application/json'
            begin
              data_hash = JSON.parse(data, {symbolize_names: true})
              handle_command_for(metadata.type, data_hash)
            rescue JSON::ParserError => e
              log.error("Error parsing json: #{e.message}")
            end
          else
            log.error("No handler for content type: #{metadata[:content_type]}")
          end
        end
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
      # Internal command handler
      # @param [Symbol] target
      # @param [Hash] data
      def handle_command_for(target, data)
        target = target.to_sym
        log.debug "Command for #{target} with: #{data}"
        @receivers[target].each do |receiver|
          if receiver.respond_to?(:handle_command)
            # TODO: validate data here!!
            # TODO: what about scheduler?
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
          else
            log.error "Listener for #{receiver} doesn't have the handle_command method!"
          end
        end
      end

      def handle_info_request(about)
        if about.to_sym == :receivers
          {receivers: @receivers.keys}
        else
          log.error "no information about '#{about}' found"
          {}
        end
      end
    end
  end
end
