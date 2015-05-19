require 'bunny'
require 'json'
require 'securerandom'
require 'timeout'
require 'singleton'

module NixonPi
  module RabbitMQ
    #Producer
    class InformationSender
      include BunnyConnection
      include Logging
      include Singleton #i didn't want to use singleton here but i can't get it to work with sinatra otherwise...

      attr_accessor :call_id
      attr_reader :lock, :condition

      def initialize
        @reply_queue = channel.queue('nixonpi.rpc.reply')
        @lock = Mutex.new
        @condition = ConditionVariable.new

        self.call_id = SecureRandom.hex(4)
        that = self

        # Register reply queue
        @reply_queue.subscribe do |delivery_info, properties, payload|
          log.debug "Got reply for #{properties[:correlation_id]}, current id: #{that.call_id} with payload: #{payload}"
          if properties[:correlation_id] == that.call_id
            log.debug 'Correlation ID match - signal lock release!'
            #It sould work with the following code line but sadly it doesn't - thats why I'm using the class variable
            #that.response = JSON.parse(payload, {symbolize_names: true})
            @@reply = JSON.parse(payload, {symbolize_names: true})
            log.error "Setting response: #{@@reply}"

            that.lock.synchronize { that.condition.signal }
          end
        end
      end

      def get_info_from(target, about = {})
        channel
            .queue('nixonpi.rpc.request')
            .publish(about.to_json,
                     correlation_id: self.call_id,
                     content_type: 'application/json',
                     type: target.to_s,
                     reply_to: @reply_queue.name,
                     routing_key: 'nixonpi.rpc.request')
        lock.synchronize { condition.wait(lock) }
        log.error "got final response: #{@@reply}"
        @@reply
      end
    end
  end
end
