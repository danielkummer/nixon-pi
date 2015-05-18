require 'bunny'
require 'json'

module NixonPi
  module RabbitMQ
    class InformationSender
      include BunnyConnection
      include Logging

      attr_reader :reply_queue
      attr_accessor :response, :call_id
      attr_reader :lock, :condition

      def initialize()
        @ch = channel
        @exchange = channel.default_exchange

        @server_queue = 'information'
        @reply_queue = channel.queue('', exclusive: true)

        @lock = Mutex.new
        @condition = ConditionVariable.new
        that = self

        @reply_queue.subscribe do |delivery_info, properties, payload|
          if properties[:correlation_id] == that.call_id
            that.response = JSON.parse(payload, {symbolize_keys: true})
            that.lock.synchronize { that.condition.signal }
          end
        end
      end

      def get_info_from(target, about)
        self.call_id = self.generate_uuid

        @exchange.publish(about.to_json,
                          routing_key: @server_queue,
                          correlation_id: self.call_id,
                          content_type: 'application/json',
                          type: target.to_s,
                          reply_to: @reply_queue.name)

        lock.synchronize { condition.wait(lock) }
        log.debug "get info: #{response}"
        response
      end

      protected
      def generate_uuid
        # very naive but good enough for code
        # examples
        "#{rand}#{rand}#{rand}"
      end
    end
  end
end
