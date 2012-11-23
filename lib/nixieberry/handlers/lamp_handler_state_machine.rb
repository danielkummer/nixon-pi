require 'state_machine'
require_relative '../../../lib/nixieberry/drivers/lamp_driver'
require_relative 'handler_state_machine'

module NixieBerry
  class LampHandlerStateMachine < HandlerStateMachine

    register_as :lamps

    def after_create
      register_driver NixieBerry::LampDriver
    end

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

      event :display_test do
        transition all => :display_test
      end


      state :display_free_value do
        def write
          lamp_values = current_state_parameters[:values]
          unless lamp_values.nil?
            if lamp_values.include? nil
              lamp_values.each_with_index { |value, index| driver.write_to_lamp(index, value) unless value.nil? }
            else
              driver.write(lamp_values)
            end
          end
        end
      end

      state :display_bar_animation do
        def write
          raise NotImplementedError
        end
      end

      state :display_test do
        def write
          raise NotImplementedError
        end
      end
    end
  end
end