require 'json'
require 'singleton'
require 'bunny'

require_relative '../logging/logging'
require_relative 'command_listener'
require_relative '../../nixonpi/hash_monkeypatch'
require_relative '../information/information_holder'

module NixonPi
  module Messaging
    module Common
      include Logging

      ##
      # RabbitMQ Client
      def client
        unless $client
          conn = Bunny.new
          conn.start
          $client = conn
        end
        $client
      end

      ##
      # Exchange channel (use direct exchange with rounting keys)
      def direct_exchange_channel
        $direct_exchange ||= client.create_channel.direct("nixonpi.channel")
      end

      ##
      # Clean up
      def on_exit
        direct_exchange_channel.delete
        #queues delete
        log.info "closing connection to rabbitmq..."
        client.close
      end
    end

    class CommandSender
      include Common

      ##
      # Send an async command to a registered commandreceiver
      # @param [Symbol] target
      # @param [Hash] payload
      def send_command(target, payload)
        log.info "sending #{payload.to_s} to #{target}"
        direct_exchange_channel.publish(payload.to_json,
                                        routing_key: "nixonpi.command",
                                        content_type: 'application/json',
                                        type: target)
      end
    end

    class CommandReceiver
      include Common
      include InformationHolder

      def initialize
        @receivers = Hash.new
        @incomming_commands_queue = client.queue("", exclusive: false, auto_delete: true).bind(direct_exchange_channel, routing_key: "nixonpi.command")
        @incomming_commands_queue.subscribe(consumer_tag: "command_consumer") do |delivery_info, metadata, data|
          if metadata[:content_type] == 'application/json'
            datahash = JSON.parse(data)
            handle_command_for(metadata.type, datahash.string_key_to_sym)
          else
            log.error("no hanlder for content type: #{metadata[:content_type]}")
          end
        end
      end

      ##
      #
      # @param [Hash] command command to listen to
      # @param [CommandListener] receiver
      def add_receiver(receiver, command)
        command = command.to_sym
        raise "Receiver must include the receiver module" unless receiver.is_a?(CommandListener)
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
            data.delete_if { |k, v| !receiver.class.available_commands.include?(k) or v.nil? }
            receiver.handle_command(data)
          else
            log.error "Listener for #{target} doesn't have the handle_command method!"
          end
        end if @receivers.has_key?(target)
      end

      def handle_info_request(about)
        ret = Hash.new
        if about.to_sym == :targets
          ret = {targets: @receivers.keys}
        else
          log.error "no information about #{about} found"
        end
        ret
      end

    end
  end
end
