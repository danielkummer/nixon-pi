require 'json'
require 'singleton'
require 'bunny'

require_relative 'client'
require_relative 'command_listener'
require_relative '../../nixonpi/hash_monkeypatch'
require_relative '../information/information_holder'

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
            data = JSON.parse(data).string_key_to_sym
            receiver = metadata.type
            handle_access(receiver) {
              handle_lock(receiver, data[:lock])
              handle_command_for(receiver, data)
            }
          else
            log.error("no hanlder for content type: #{metadata[:content_type]}")
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
      # @param [CommandListener] receiver
      def add_receiver(receiver, command)
        command = command.to_sym
        raise "Receiver #{receiver.class} must include the receiver module" unless receiver.is_a?(CommandListener)
        if @receivers[command].nil?
          @receivers[command] = [receiver]
        else
          @receivers[command] << receiver
        end

      end

      ##
      # Internal command handler
      # @param [Symbol] receiver
      # @param [Hash] data
      def handle_command_for(receiver, data)
        receiver = receiver.to_sym
        @receivers[receiver].each do |receiver|
          if receiver.respond_to?(:handle_command)
            data.delete_if { |k, v| !receiver.class.available_commands.include?(k) or v.nil? }
            receiver.handle_command(data)
          else
            log.error "Listener for #{receiver} doesn't have the handle_command method!"
          end
        end if @receivers.has_key?(receiver)
      end

      def handle_info_request(about)
        ret = Hash.new
        if about.to_sym == :receivers
          ret = {receivers: @receivers.keys}
        else
          log.error "no information about #{about} found"
        end
        ret
      end

    end
  end
end
