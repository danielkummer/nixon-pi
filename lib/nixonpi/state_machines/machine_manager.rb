require_relative 'handler_state_machine'
require_relative '../logging/logging'

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
      # @param [Integer] instances_count number of instances to create, this adds a numeric suffix to the instances queue listeners (ex: lamp0, lamp1, lamp2,...)
      # @param [Symbol] name add a state machines to the manager, the machines added must have a corresponding type in the factory module (a machine registers itself using the register_as class method.)
      def add_state_machine(name, instances_count = 1)
        instances_count.times.with_index do |i|
          suffix = instances_count == 1 ? "" : i.to_s
          key = "#{name}#{suffix}".to_sym
          log.debug "adding state machine instance for #{name} under #{key}"
          @@state_machines[key] = NixonPi::HandlerStateMachine.create(key)
        end

      end


      ##
      # Start each state machine in a separate thread
      # @param [Float] sleep_for_sec sleep time after each loop
      def start_state_machines(sleep_for_sec = 0.3)
        @@state_machines.each do |type, state_machine|
          log.info "Ading state machine #{state_machine.class} to the command processor as type #{type}"

          NixonPi::CommandQueue.add_receiver(state_machine, type)

          log.info "Starting state machine: #{state_machine.class}"
          @@threads << Thread.new do
            loop do
              state_machine.handle
              sleep sleep_for_sec #tacted at 100ms, adjust if necessary
            end
          end
        end
      end

      def get_params_for(type)
        instance = @@state_machines[type.to_sym]
        instance.get_params.clone unless instance.nil?
      end

      ##
      # Join the state machine threads
      def join_threads
        @@threads.each { |thread| thread.join }
      end
    end
  end
end