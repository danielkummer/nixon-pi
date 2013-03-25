require 'state_machine'

require_relative '../drivers/basic/pwm_driver'
require_relative 'base_state_machine'
require_relative '../animations/animation'
require_relative '../../nixonpi/animations/bar/ramp_up_down_animation'
require_relative '../configurations/settings'
require_relative '../../dependency'


module NixonPi
  class BarStateMachine < BaseStateMachine

    register :bar, self
    accepted_commands :state, :value, :animation_name, :options


    def initialize()
      super()
      register_driver get_injected(:in13_tubes)
    end

    state_machine do

      state :startup do
        def write
          get_injected(:ramp_up_down, {bar: bar_index}).run("0")
          if params[:initial_state].nil?
            params[:goto_state] = :free_value
            params[:value] = 0
          else
            params[:goto_state] = params[:initial_state] #todo load initial values
          end

          handle_command(state: params[:goto_state])
        end
      end

      state :free_value do
        def write
          value = params[:value]
          if !value.nil? and value != params[:last_value]
            @driver.write_to_port(bar_index, value)
            params[:last_value] = value
          end

        end
      end
    end

    def bar_index
      registered_as_type.to_s.match(/bar(\d+)/)[1]
      $1.to_i
    end

  end

end