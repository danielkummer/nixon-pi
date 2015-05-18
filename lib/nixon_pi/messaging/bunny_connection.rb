require 'bunny'

module NixonPi
  module Messaging
    module BunnyConnection
      include Logging

      ##
      # RabbitMQ Client
      # @raise [Bunny::TCPConnectionFailed] if server not reachable
      def client
        unless @client
          begin
            conn = Bunny.new
            @client = conn.start
          rescue Bunny::TCPConnectionFailed => e
            # TODO: properly handle the connection error
            log.error "Failed to connect to Rabbitmq: #{e.message}"
            raise e
          end
        end
        @client
      end

      def channel
        @channel ||= client.create_channel
      end

      def topic(type)
        case type
          when :command
            channel.topic('topic_commands')
          when :request
            channel.topic('topic_request')
          when :response
            channel.topic('topic_response')
          else
            fail "unkown topic: #{type}"
        end
      end

      def connected?
        client.connected?
        true
      rescue Exception => e
        log.error "can't connect to rabbitmq: #{e.message}"
        false
      end

      ##
      # Clean up
      def on_exit
        @client.close
      end
    end
  end
end
