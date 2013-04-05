require 'state_machine'
require_relative '../drivers/proxies/lamp_proxy'
require_relative 'base_state_machine'
require_relative '../configurations/settings'
require_relative '../../dependency'
require_relative '../animations/lamp/blink_animation'

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
          #transition over to the animation state after setting the correct values
          goto_state = params[:initial_state].nil? ? :free_value : params[:initial_state]
          params[:animation_name] = :blink
          params[:options] = {lamp:lamp_index, goto_state: goto_state, goto_target: "lamp#{lamp_index}".to_sym}
          handle_command(state: :animation)
        end
      end

    end

    def lamp_index
      registered_as_type.to_s.match(/lamp(\d+)/)[1]
      $1.to_i
    end
  end
end