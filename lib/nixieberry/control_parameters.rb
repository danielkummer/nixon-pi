module NixieBerry
  module ControlParameters

    ##
    # Tube control parameters to be passed via the command queue
    # only use the ones you need...
    #Todo refactor!
    def control_parameters(handler)
      case handler
        when :tubes
          {
              mode: nil,
              value: nil,
              time_format: nil,
              animation_name: nil,
              animation_options: nil,
              time: nil
          }
        when :bars
          {
              #pass values as array containing nil where no change
              values: nil,
              time: nil
          }
        when :lamps
          {
            values: nil,
            time: nil
          }
        else
          raise NotImplementedError "Unknown control parameters -- implement if new function"
      end
    end
  end
end