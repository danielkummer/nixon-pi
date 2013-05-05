require_relative 'base_state_machine'
require_relative '../logging/logging'
require_relative '../messaging/command_receiver'
require_relative '../messaging/commands_module'

module NixonPi
  class MultiMachineProxy
    extend Logging
    extend Commands

    #accepted_commands :value
    # :state, :value, :animation_name, :options

    def initialize
      @@state_machines = Set.new

    end

    def handle_command(command)
      #pass to underlying state machines
      #split values to give to every state machine

    end

    def add_state_machine(state_machine)
      self.class.accepted_commands = self.class.accepted_commands + state_machine.class.accepted_commands
      @@state_machines << state_machine
    end

  end
end
