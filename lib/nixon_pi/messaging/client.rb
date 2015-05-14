require 'bunny'

module NixonPi
  module Messaging
    module Client
      include Logging

      ##
      # RabbitMQ Client
      # @raise [Bunny::TCPConnectionFailed] if server not reachable
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
        $direct_exchange ||= client.create_channel.direct('nixonpi.channel')
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
        # queues delete
        #log.info 'closing connection to rabbitmq...'
        client.close
      end
    end
  end
end
