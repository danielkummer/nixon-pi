require_relative 'handler_state_machine'
require_relative '../logging/logging'

module NixonPi
  class MachineManager
    extend Logging

    @@state_machines, @@threads = [], []

    class << self

      ##
      # Add a number of state machines to the manager
      def add_state_machines(*types)
        log.info "Adding state machines: #{types.to_s}"
        types.each do |type|
          @@state_machines << NixonPi::HandlerStateMachine.create(type)
        end
      end

      ##
      # Start each state machine in a separate thread
      # @param [Float] sleep_for_sec sleep time after each loop
      def start_state_machines(sleep_for_sec = 0.3)
        @@state_machines.each do |state_machine|
          log.info "Starting state machine: #{state_machine.class}"
          @@threads << Thread.new do
            loop do
              state_machine.handle
              sleep sleep_for_sec #tacted at 100ms, adjust if necessary
            end
          end
        end
      end

      ##
      # Join the state machine threads
      def join_threads
        @@threads.each { |thread| thread.join }
      end
    end
  end
end