require 'state_machine'
require_relative '../drivers/lamp_driver'
require_relative 'base_state_machine'
require_relative '../configurations/settings'
require_relative '../drivers/driver_manager'

module NixonPi
  class LampStateMachine <  BaseStateMachine

    register_as :lamp
    accepted_commands :state, :value, :animation_name, :options

    def initialize()
      super()
      register_driver DriverManager.instance_for(:in1)
    end

    state_machine do


      state :free_value do
        def write
          value = params[:value]
          if !value.nil? and value != params[:last_value]
            @driver.write_to_lamp(lamp_index, value)
            params[:last_value] = value
          end

        end
      end


      state :startup do
        def write
          #do some startup animation stuff....
          #todo refactor
          params[:value] = 0

=begin
          if params[:initial_state].nil?
            self.fire_state_event(:free_value)
          else
            self.fire_state_event(params[:initial_state])
          end
=end

          handle_command(state: :free_value, value: 0)
        end
      end

    end

    def lamp_index
      registered_as_type.to_s.match(/lamp(\d+)/)[1]
      $1.to_i
    end
  end
end