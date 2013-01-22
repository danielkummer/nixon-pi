require_relative 'command_queue'
require_relative 'logging/logging'


module NixonPi
  class CommandProcessor
    extend Logging

    @@listeners = {}
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

          if command[:time] and command[:time] + 2 > Time.now #do nothing if command is older than 2 seconds
            log.debug "processing command: #{command} in queue #{queue_name}, checking for invalid control parameters..."
            @@listeners[queue_name].each do |listener|

              #todo there might be a better place to handle invalid commands, maybe when enqueuing them?
              command.delete_if { |k, v| !listener.class.available_commands.include?(k) or v.nil? }

              if listener.respond_to?(:receive)
                listener.try(:receive, command)
                #todo add rescue (maybe)
              else
                log.error "Listener for #{queue_name} doesn't have the receive method!"
              end
            end
          end
        end

      end
    end

  end
end