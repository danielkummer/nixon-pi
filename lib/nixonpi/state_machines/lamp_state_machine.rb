require 'state_machine'
require_relative '../../../lib/nixonpi/drivers/lamp_driver'
require_relative 'handler_state_machine'

module NixonPi
  class LampStateMachine < HandlerStateMachine

    register_as :lamp0, :lamp1, :lamp2, :lamp3, :lamp4

    def after_create
      register_driver NixonPi::LampDriver
      #reload_from_db(:lamps)
      CommandProcessor.add_receiver(self, registered_as_type)
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
            driver.write_to_lamp(lamp_index, value)
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

    def lamp_index
      registered_as_type.to_s.match(/lamp(\d+)/)[1]
      $1.to_i
    end
  end
end