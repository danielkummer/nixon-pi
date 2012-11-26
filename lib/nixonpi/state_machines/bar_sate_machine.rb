require 'state_machine'

require_relative '../drivers/bar_graph_driver'
require_relative 'handler_state_machine'
require_relative '../animations/animation'
require_relative '../../nixonpi/animations/bar/ramp_up_down_animation'


module NixonPi
  class BarStateMachine < HandlerStateMachine

    register_as :bars

    def after_create
      register_driver NixonPi::BarGraphDriver
    end

    state_machine :initial => :display_test do

      around_transition do |object, transition, block|
        HandlerStateMachine.handle_around_transition(object, transition, block)
      end

      event :display_free_value do
        transition all => :display_free_value
      end


      event :display_bar_animation do
        puts "in display_bar_animation"
        transition all => :display_bar_animation
      end

      event :display_test do
        transition all => :display_test
      end


      state :display_free_value do
        def write
          bar_values = current_state_parameters[:values]
          if !bar_values.nil? and values_changed?(bar_values)
            if bar_values.include? nil
              bar_values.each_with_index { |value, index| driver.write_to_bar(index, value) unless value.nil? }
            else
              driver.write(bar_values)
            end
          end
        end
      end

      state :display_bar_animation do
        def write
          animation_name = current_state_parameters[:animation_name]
          animation_options = current_state_parameters[:animation_options]
          animation_options ||= {}
          start_value = current_state_parameters[:last_value]
          NixonPi::Animations::Animation.create(animation_name.to_sym, animation_options).run(start_value)
          self.send(current_state_parameters[:last_state]) #go back to old state again and do whatever was done before
        end
      end

      state :display_test do
        def write
          #careful, endless loop!!
          #NixieBerry::Animations::Animation.create(:ramp_up_down).run
          self.fire_state_event(:display_free_value)
        end
      end
    end

  end

end