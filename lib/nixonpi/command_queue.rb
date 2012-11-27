require 'thread'
require_relative 'control_parameters'


module NixonPi
  class CommandQueue
    include ControlParameters

    @@queues = {}

    class << self

      ##
      # Enqueue a new command
      # @param [Symbol] worker
      # @param [Hash] params
      def enqueue(worker, params)
        command = control_parameters(worker).merge(params)
        command[:time] = Time.now
        queue(worker) << command
      end

      ##
      # Get a worker queue
      # @param [Symbol] worker
      def queue(worker)
        @@queues[worker] ||= Queue.new
      end

    end
  end
end