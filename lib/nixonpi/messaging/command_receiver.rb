require 'json'
require 'singleton'
require 'bunny'

require_relative 'client'
require_relative 'commands_module'
require_relative '../../nixonpi/hash_monkeypatch'
require_relative '../information/information_holder'
require_relative '../../../web/models'

module NixonPi
  module Messaging
    class CommandReceiver
      include Client
      include InformationHolder

      def initialize
        @receivers = Hash.new
        @locks = Set.new
        @incomming_commands_queue = client.queue("", exclusive: false, auto_delete: true).bind(direct_exchange_channel, routing_key: "nixonpi.command")
        @incomming_commands_queue.subscribe(consumer_tag: "command_consumer") do |delivery_info, metadata, data|
          if metadata[:content_type] == 'application/json'
            #todo handle parse exception
            begin
              data_hash = JSON.parse(data).string_key_to_sym
            rescue Exception => e
              log.error(e.message)
            end


            receiver = metadata.type
            handle_access(receiver) {
              handle_lock(receiver, data_hash[:lock])

              handle_command_for(receiver, data_hash)
            }
          else
            log.error("no handler for content type: #{metadata[:content_type]}")
          end
        end
      end

      ##
      # Handle lock for a specified receiver
      # @param [Symbol] receiver
      # @param [Boolean] lock
      def handle_lock(receiver, lock)
        return if lock.nil?
        case lock.to_sym
          when :lock
            lock(receiver)
          when
          unlock(receiver)
        end
      end


      def locked?(receiver)
        @locks.include?(receiver)
      end

      def lock(receiver)
        @locks << receiver
      end

      def unlock(receiver)
        @locks.delete_if { |l| l == receiver }
      end

      ##
      # Handle access for the message queue
      # @param [Symbol] receiver receiver
      def handle_access(receiver)
        if locked?(receiver)
          log.error "Queue #{receiver} is locked!"
        else
          yield
        end
      end

      ##
      #
      # @param [Hash] command command to listen to
      # @param [Commands] receiver
      def add_receiver(receiver, command)
        command = command.to_sym
        raise "Receiver #{receiver.class} must include the receiver module" unless receiver.is_a?(Commands)
        if @receivers[command].nil?
          @receivers[command] = [receiver]
        else
          @receivers[command] << receiver
        end

      end

      ##
      # Internal command handler
      # @param [Symbol] target
      # @param [Hash] data
      def handle_command_for(target, data)
        target = target.to_sym
        @receivers[target].each do |receiver|
          if receiver.respond_to?(:handle_command)
            #todo validate data here!!
            #todo what about scheduler?
            model_data = data.clone
            model_data[:target] = target
            model_data.delete(:id)
            #todo locking here isn't good
            validator_model = target == :schedule ? Schedule.new(model_data) : Command.new(model_data)
            if validator_model.valid?
              data.delete_if { |k, v| !receiver.class.available_commands.include?(k) or v.nil? }
              receiver.handle_command(data)
            else
              log.error("invalid command received: #{data.to_s}, errors: #{validator_model.errors.full_messages.join("\n")}")
            end
          else
            log.error "Listener for #{receiver} doesn't have the handle_command method!"
          end
        end if @receivers.has_key?(target)
      end

      def handle_info_request(about)
        ret = Hash.new
        if about.to_sym == :receivers
          ret = {receivers: @receivers.keys}
        else
          log.error "no information about '#{about}' found"
        end
        ret
      end

    end
  end
end
