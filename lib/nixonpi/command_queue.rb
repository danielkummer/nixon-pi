require 'thread'
require_relative 'logging/logging'


module NixonPi
  class CommandQueue
    extend Logging

    @@queues = {}
    @@locked = {}

    class << self

      ##
      # Enqueue a new command
      # @param [Symbol] worker
      # @param [Hash] params
      def enqueue(worker, params)
        worker = worker.to_sym
        command = params.clone
        if can_enqueue?(worker, command)
          command[:time] = Time.now
          log.info "Enqueueing #{command.to_s} for #{worker}"
          queue(worker) << command
        else
          log.error "Queue currently locked, cannot enqueue #{params.to_s} for #{worker}"
        end
      end

      def lock(worker)
        @@locked[worker] = true
      end

      def unlock(worker)
        @@locked[worker] = false
      end

      def locked?(worker)
        @@locked[worker] ? true : false
      end

      ##
      # Get a worker queue
      # @param [Symbol] worker
      def queue(worker)
        @@queues[worker] ||= Queue.new
      end

      private
      def can_enqueue?(worker, params)
        if locked?(worker) and params[:priority] == true
          true
        elsif locked?(worker)
          false
        else
          true
        end
      end
    end
  end
end