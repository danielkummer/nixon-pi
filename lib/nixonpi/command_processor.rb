require_relative 'command_queue'
require_relative 'logging/logging'




module NixonPi
  class CommandProcessor
    extend Logging

    @@thread = nil

    class << self
      def start
        @@thread = Thread.new do
          loop do
            process_queues
            sleep 0.3
          end
        end
      end

      def exit
        Thread.kill @@thread
      end

      def add_receiver(receiver, queue)
        raise "Receiver must include the receiver module" unless receiver.is_a?(CommandReceiver)

        if @@listeners[queue.to_sym].nil?
          @@listeners[queue.to_sym] = [receiver]
        else
          @@listeners[queue.to_sym] << receiver
        end
      end

      def get_receiver_for(queue)
        @@listeners[queue.to_sym]
      end

      def join_thread
        @@thread.join
      end

      private
      def process_queues
        @@listeners.keys.each do |queue_name|
          process_queue(queue_name)
        end
      end

      def process_queue(queue_name)
        queue = CommandQueue.queue(queue_name)
        unless queue.empty?
          command = queue.pop


        end

      end
    end

  end
end