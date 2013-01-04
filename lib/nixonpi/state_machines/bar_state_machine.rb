require 'state_machine'

require_relative '../drivers/bar_graph_driver'
require_relative 'handler_state_machine'
require_relative '../animations/animation'
require_relative '../../nixonpi/animations/bar/ramp_up_down_animation'
require_relative '../command_processor'


module NixonPi
  class BarStateMachine < HandlerStateMachine

    register_as :bars

    def after_create
      register_driver NixonPi::BarGraphDriver
      reload_from_db(:bars)
      CommandProcessor.add_receiver(self, :bars)
    end

    state_machine :initial => :startup do

      around_transition do |object, transition, block|
        HandlerStateMachine.handle_around_transition(object, transition, block)
      end

      event :free_value do
        transition all => :free_value
      end


      event :animation do
        puts "in animation"
        transition all => :animation
      end

      state :startup do
        def write
          current_state_parameters[:animation_name] = "ramp_up_down"
          current_state_parameters[:options] = ""
          current_state_parameters[:last_value] = "0000"

          #will switch to :last_state after animation
          self.fire_state_event(:animation)
          #unlucky naming - currently :state is a saved db value - if any; reason: no state transition has happened yet
          if !current_state_parameters[:initial_state].nil?
            current_state_parameters[:last_state] = current_state_parameters[:initial_state]
          else
            current_state_parameters[:last_state] = :free_value
          end


        end
      end

      event :run_test do
        transition all => :run_test
      end


      state :free_value do
        def write
          bar_values = current_state_parameters[:values]
          if !bar_values.nil? and values_changed?(bar_values)
            if bar_values.include? nil
              bar_values.each_with_index { |value, index| driver.write_to_bar(index, value) unless value.nil? }
            else
              driver.write(bar_values)
              current_state_parameters[:last_values] = bar_values
            end
          end
        end
      end

      state :animation do
        def write
          name, options = current_state_parameters[:animation_name], current_state_parameters[:options]
          options ||= {}
          start_value = current_state_parameters[:last_value]
          NixonPi::Animations::Animation.create(name.to_sym, options).run(start_value)
          self.send(current_state_parameters[:last_state]) #go back to old state again and do whatever was done before
        end
      end

      state :run_test do
        def write
          #careful, endless loop!!
          #NixieBerry::Animations::Animation.create(:ramp_up_down).run
          self.fire_state_event(:free_value)
        end
      end
    end

  end

end