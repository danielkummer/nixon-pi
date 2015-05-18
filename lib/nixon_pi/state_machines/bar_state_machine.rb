require 'state_machine'

module NixonPi
  class BarStateMachine < BaseStateMachine
    include NixonPi::DependencyInjection

    register :bar, self
    accepted_commands :state, :value, :animation_name, :options

    def initialize
      super()
      register_driver self.class.get_injected(:in13_driver)
    end

    state_machine do
      state :startup do
        def write
          # transition over to the animation state after setting the correct values
          goto_state = params[:initial_state].nil? ? :free_value : params[:initial_state]

          params[:animation_name] = :ramp_up_down
          params[:options] = { bar: bar_index, goto_state: goto_state, goto_target: "bar#{bar_index}".to_sym }
          handle_command(state: :animation)
        end
      end

      state :free_value do
        def write
          value = params[:value]
          if !value.nil? && value != params[:last_value]
            @driver.write_to_port(bar_index, value)
            params[:last_value] = value
          end
        end
      end
    end

    def bar_index
      registered_as_type.to_s.match(/bar(\d+)/)[1]
      Regexp.last_match(1).to_i
    end
  end
end
