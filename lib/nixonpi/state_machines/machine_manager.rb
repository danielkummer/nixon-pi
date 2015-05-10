require_relative 'base_state_machine'
require_relative '../logging/logging'
require_relative '../messaging/command_receiver'

module NixonPi
  class MachineManager
    extend Logging

    @@state_machines = {}
    @@threads = []

    class << self
      ##
      # exit threads
      def exit
        @@threads.each do |t|
          Thread.kill(t)
        end
      end

      ##
      # Add a number of state machines to the manager
      # @param [Symbol] name add a state machines to the manager, the machines added must have a corresponding type in the factory module (a machine registers itself using the register_as class method.)
      # @param [Integer] instances_count number of instances to create, this adds a numeric suffix to the instances queue listeners (ex: lamp0, lamp1, lamp2,...)
      def add_state_machines(name, instances_count = 1)
        instances_count.times.with_index do |i|
          suffix = instances_count == 1 ? '' : i.to_s
          key = "#{name}#{suffix}".to_sym
          instance = get_injected(key)
          @@state_machines[key] = instance
          yield instance, key if block_given?
          log.debug "Added state machine for #{name} under key #{key}"
        end
      end

      ##
      # Start each state machine in a separate thread
      # @param [Float] sleep_for_sec sleep time after each loop
      def start_state_machines(sleep_for_sec = 0.1)
        @@state_machines.each do |_type, state_machine|
          log.debug "Starting state machine: #{state_machine.class}"
          @@threads << Thread.new do
            loop do
              state_machine.handle
              sleep sleep_for_sec
            end
          end
        end
      end

      ##
      # Join the state machine threads
      def join_threads
        @@threads.each { |thread| thread.join unless thread.nil? }
      end
    end
  end
end
