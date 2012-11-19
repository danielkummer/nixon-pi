require 'state_machine'
require 'singleton'
require_relative '../../../lib/nixieberry/drivers/lamp_driver'
require_relative 'handler_state_machine'

module NixieBerry
  class LampHandlerStateMachine < HandlerStateMachine
    include Singleton

    register_driver NixieBerry::LampDriver.instance
    register_queue_name :lamps


    state_machine :initial => :display_free_value do

      around_transition do |object, transition, block|
        handle_around_transition(object, transition, block)
      end

      event :display_free_value do
        transition all => :display_free_value
      end


      event :display_lamp_animation do
        transition all => :display_lamp_animation
      end

      event :test do
        transition all => :test
      end


      state :display_free_value do
        def write
          lamp_values = @current_state_parameters[:values]
          unless values_changed?(lamp_values)
            if lamp_values.include? nil
              lamp_values.each_with_index { |value, index| @driver.write_to_lamp(index, value) unless value.nil? }
            else
              @driver.write(lamp_values)
            end
          end
        end
      end

      state :display_bar_animation do
        def write
          raise NotImplementedError
        end
      end

      state :test do
        def write
          raise NotImplementedError
        end
      end
    end
  end
end