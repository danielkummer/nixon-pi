require 'state_machine'
require_relative '../drivers/proxies/lamp_proxy'
require_relative 'base_state_machine'
require_relative '../configurations/settings'
require_relative '../../dependency'

module NixonPi
  class LampStateMachine <  BaseStateMachine

    register :lamp, self
    accepted_commands :state, :value, :animation_name, :options

    def initialize()
      super()
      register_driver get_injected(:in1_proxy)
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