require 'bunny'

module NixonPi
  module RabbitMQ
    module BunnyConnection
      include Logging

      ##
      # RabbitMQ Client
      # @raise [Bunny::TCPConnectionFailed] if server not reachable
      # @raise [Exception] more generic exception, see bunny documentation for details
      def client
         @client ||= begin
          begin
            conn = Bunny.new(read_timeout: 2, heartbeat: 2)
            @client = conn.start
          rescue Bunny::TCPConnectionFailed => e
            # TODO: properly handle the connection error
            log.error "Failed to connect to Rabbitmq: #{e.message}"
            raise e
          rescue Exception => e
            log.error "Failed; #{e.message}"
            raise e
          end
        end
      end

      ##
      # Get or create a channel
      #
      # @return [Bunny::Channel] channel
      def channel
        @channel ||= client.create_channel
      end

      ##
      # Create a new topic exchange
      #
      # @param [Symbol] type the topic type, (:command, :request, :response)
      # @return [Exchange] exchange
      def topic(type)
        case type
          when :command
            channel.topic('topic_commands')
          when :request
            channel.topic('topic_request')
          when :response
            channel.topic('topic_response')
          else
            fail "Unknown topic: #{type}"
        end
      end

      ##
      # Check if the client is connected
      #
      # @return [Boolean] true if connected
      def connected?
        client.connected?
        true
      rescue Exception => e
        log.error "Can't connect to rabbitmq: #{e.message}"
        false
      end

      ##
      # Clean up - call this method when exiting the application
      def on_exit
        client.close
      end
    end
  end
end
