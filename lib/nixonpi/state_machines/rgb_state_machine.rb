require 'state_machine'
require_relative '../drivers/lamp_driver'
require_relative 'handler_state_machine'
require_relative '../configurations/settings'
require_relative '../drivers/driver_manager'

module NixonPi
  class RgbStateMachine <  HandlerStateMachine

    register_as :rgb
    accepted_commands :state, :value, :animation_name, :options

    def initialize()
      super()
      register_driver DriverManager.driver_for(:rgb)
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

      state :free_value do
        def write
          value = params[:value]
          if !value.nil? and value != params[:last_value]
            driver.write(value)
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
          params[:value] = 0
          handle_command(state: :free_value, value: 0)
        end
      end

    end
  end
end