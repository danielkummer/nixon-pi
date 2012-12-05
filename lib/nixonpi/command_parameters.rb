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
              mode: nil,
              value: nil,
              time_format: nil,
              animation_name: nil,
              options: nil,
              time: nil
          }
        when :bars
          {
              mode: nil,
              values: nil,
              time_format: nil,
              animation_name: nil,
              options: nil,
              time: nil
          }
        when :lamps
          {
              mode: nil,
              values: nil,
              time_format: nil,
              animation_name: nil,
              options: nil,
              time: nil
          }
        when :say
          {
              value: nil
          }
        else
          raise NotImplementedError "Unknown control parameters -- implement if new function"
      end
    end
  end
end