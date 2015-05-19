require 'bunny'
require 'json'
require 'securerandom'
require 'timeout'
require 'singleton'

module NixonPi
  module RPC
    ##
    # Producer of information requests
    class InformationSender
      include BunnyConnection
      include Logging
      include Singleton # I use singleton here to prevent sinatra threading errors (it works together with set :lock)

      attr_accessor :call_id
      attr_reader :lock, :condition

      def initialize
        @lock = Mutex.new
        @condition = ConditionVariable.new

        self.call_id = SecureRandom.hex(4)

        that = self

        @reply_queue = channel.queue(named_queue(:reply))
        # Register reply queue
        @reply_queue.subscribe do |delivery_info, properties, payload|
          if properties[:correlation_id] == that.call_id
            log.debug 'Correlation ID match - signal lock release!'
            #It should work with the following code line but sadly it doesn't - that'ss why I'm using the class variable
            #that.response = JSON.parse(payload, {symbolize_names: true})
            @@reply = JSON.parse(payload, {symbolize_names: true})
            log.error "Setting response: #{@@reply}"

            that.lock.synchronize { that.condition.signal }
          end
        end
      end

      ##
      # RPC! Send information request to a target
      #
      # @param [Symbol] target the receiver for the information request
      # @param [Hash] about hash containing specific information requeset data
      def send_information_request(target, about = {})
        channel
            .queue(named_queue(:request))
            .publish(about.to_json,
                     correlation_id: self.call_id,
                     content_type: 'application/json',
                     type: target.to_s,
                     reply_to: @reply_queue.name,
                     routing_key: 'nixonpi.rpc.request')
        lock.synchronize { condition.wait(lock) }
        log.debug "Received response from remote: #{@@reply}"
        @@reply
      end
    end
  end
end
