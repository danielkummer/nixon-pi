require_relative 'command_queue'
require_relative 'command_parameters'
require_relative 'logging/logging'


module NixonPi
  class CommandProcessor
    extend CommandParameters
    include CommandParameters
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
        if @@listeners[queue.to_sym].nil?
          @@listeners[queue.to_sym] = [receiver]
        else
          @@listeners[queue.to_sym] << receiver
        end
      end

      def join_thread
        @@thread.join
      end

      def process_queues
        @@listeners.keys.each do |queue_name|
          process_queue(queue_name)
        end
      end

      def process_queue(queue_name)
        queue = CommandQueue.queue(queue_name)
        unless queue.empty?
          command = queue.pop
          log.debug("Got command: #{command} in queue #{queue_name}, checking for invalid control parameters...")
          command.delete_if { |k, v| !command_parameters(queue_name).keys.include?(k) or v.nil? }
          if command[:time] and command[:time] + 2 > Time.now #do nothing if command is older than 2 seconds
            @@listeners[queue_name].each do |listener|
              begin
                listener.try(:receive, command)
              rescue Exception => e
                log.debug("Listener #{listener} doesn't have the receive method!")
              end
            end
          end
        end

      end
    end

  end
end