require 'state_machine'
require_relative '../../../lib/nixonpi/drivers/lamp_driver'
require_relative 'handler_state_machine'

module NixonPi
  class LampStateMachine < HandlerStateMachine

    register_as :lamps

    def after_create
      register_driver NixonPi::LampDriver
      reload_from_db(:lamps)
      CommandProcessor.add_receiver(self, :lamps)
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
          lamp_values = params[:values]
          unless lamp_values.nil?
            if lamp_values.include? nil
              lamp_values.each_with_index { |value, index| driver.write_to_lamp(index, value) unless value.nil? }
            else
              driver.write(lamp_values)
              params[:last_values] = lamp_values
            end
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
          if params[:initial_state].nil?
            self.fire_state_event(:free_value)
          else
            self.fire_state_event(params[:initial_state])
          end
        end
      end

      state :run_test do
        def write
          raise NotImplementedError
        end
      end
    end
  end
end