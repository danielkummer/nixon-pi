require 'state_machine'

require_relative '../drivers/bar_graph_driver'
require_relative 'handler_state_machine'
require_relative '../animations/animation'
require_relative '../../nixonpi/animations/bar/ramp_up_down_animation'
require_relative '../configurations/settings'


module NixonPi
  class BarStateMachine < HandlerStateMachine

    register_as :bar
    accepted_commands :state, :value, :animation_name, :options


    def initialize()
      super()
      register_driver NixonPi::BarGraphDriver
    end

    state_machine :initial => :startup do

      around_transition do |object, transition, block|
        HandlerStateMachine.handle_around_transition(object, transition, block)
      end

      event :free_value do
        transition all => :free_value
      end


      event :animation do
        transition all => :animation
      end

      state :startup do
        def write
          #unlucky naming - currently :state is a saved db value - if any; reason: no state transition has happened yet
          if params[:initial_state].nil?
            params[:goto_state] = :free_value
          else
            params[:goto_state] = params[:initial_state]
          end

          handle_command({state:"animation", animation_name: "ramp_up_down", options: { bar: bar_index }, last_value: 0})

        end
      end

      event :run_test do
        transition all => :run_test
      end


      state :free_value do
        def write
          value = params[:value]
          if !value.nil? and value != params[:last_value]
            driver.write_to_bar(bar_index, value)
            params[:last_value] = value
          end

        end
      end

      state :animation do
        def write
          name, options = params[:animation_name], params[:options]
          options ||= {}
          start_value = params[:last_value]
          NixonPi::Animations::Animation.create(name.to_sym, options).run(start_value)
          handle_command(state: params[:goto_state])
        end
      end

    end

    def bar_index
      registered_as_type.to_s.match(/bar(\d+)/)[1]
      $1.to_i
    end

  end

end