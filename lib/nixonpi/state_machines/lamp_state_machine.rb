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

      state :blink do
        def enter_state
          @last_blink_time = Time.now
          @blink_value = 0
        end

        def write
          current_blink_time = Time.now
          @blink_value = (@blink_value == 0 ? 1 : 0) if (current_blink_time - @last_blink_time > 1)
          @driver.write_to_lamp(lamp_index, @blink_value)
          @last_blink_time = current_blink_time
        end

        def leave_state
          @driver.write_to_lamp(lamp_index, 0)
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