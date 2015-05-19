require 'bunny'

module NixonPi
  module RPC
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
            log.error "Failed to connect to RabbitMQ: #{e.message}"
            raise e
          rescue Exception => e
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
      # Get the command exchange
      def exchange(type)
        case type
          when :command
            channel.topic('exchange.nixonpi.rpc.command')
          when :request
            channel.topic('exchange.nixonpi.rpc.reqest')
          when :reply
            channel.topic('exchange.nixonpi.rpc.reply')
          else
            raise 'Unknown exchange'
        end
      end

      def named_queue(type)
        case type
          when :command
            'nixonpi.rpc.command'
          when :reply
            'nixonpi.rpc.reply'
          when :request
            'nixonpi.rpc.request'
          else
            raise 'Unknown queue name'
        end
      end



      ##
      # Check if the client is connected
      #
      # @return [Boolean] true if connected
      def connected?
        client.connected?
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
