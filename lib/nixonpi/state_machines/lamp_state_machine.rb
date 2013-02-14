require 'state_machine'
require_relative '../../../lib/nixonpi/drivers/io_driver'
require_relative 'handler_state_machine'
require_relative '../configurations/settings'

module NixonPi
  class LampStateMachine < HandlerStateMachine

    register_as :lamp
    accepted_commands :state, :value, :animation_name, :options

    def initialize()
      super()
      register_driver NixonPi::IODriver.new(Settings.in1_pins)
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

      event :run_test do
        transition all => :run_test
      end

      state :free_value do
        def write
          value = params[:value]
          if !value.nil? and value != params[:last_value]
            driver.write_to_port(lamp_index, value)
            params[:last_value] = value
          end

        end
      end

      state :animation do
        def write
          raise NotImplementedError
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

      state :run_test do
        def write
          raise NotImplementedError
        end
      end
    end

    def lamp_index
      registered_as_type.to_s.match(/lamp(\d+)/)[1]
      $1.to_i
    end
  end
end