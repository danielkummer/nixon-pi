module NixonPi
  module CommandParameters

    ##
    # Tube control parameters to be passed via the command queue
    # only use the ones you need...
    # @param [Symbol] state_machine the handling state machine
    def command_parameters(state_machine)
      case state_machine
        when :tubes
          {
              state: nil,
              value: nil,
              time_format: nil,
              animation_name: nil,
              options: nil,
              time: nil,
              initial_mode: nil
          }
        when :bars
          {
              state: nil,
              values: nil,
              animation_name: nil,
              options: nil,
              time: nil,
              initial_mode: nil
          }
        when :lamps
          {
              state: nil,
              values: nil,
              animation_name: nil,
              options: nil,
              time: nil,
              initial_mode: nil
          }
        when :say
          {
              value: nil,
              time: nil
          }
        when :power
          {
              value: nil,
              time: nil
          }
        when :schedule
          {
              id: nil,
              timing: nil,
              time: nil,
              state_machine: nil,
              command: nil
          }
        else
          raise NotImplementedError "Unknown control parameters -- implement if new function"
      end
    end
  end
end