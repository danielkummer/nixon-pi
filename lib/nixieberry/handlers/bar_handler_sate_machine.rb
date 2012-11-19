require 'json'
require 'state_machine'
require 'singleton'
require_relative '../drivers/bar_graph_driver'
require_relative 'handler_state_machine'

module NixieBerry
  class BarHandlerStateMachine < HandlerStateMachine
    include Singleton

    register_driver NixieBerry::BarGraphDriver.instance
    register_queue_name :bars


    state_machine :initial => :display_free_value do

      around_transition do |object, transition, block|
        handle_around_transition(object, transition, block)
      end

      event :display_free_value do
        transition all => :display_free_value
      end


      event :display_bar_animation do
        transition all => :display_bar_animation
      end

      event :test do
        transition all => :test
      end


      state :display_free_value do
        def write
          bar_values = @current_state_parameters[:values]
          unless values_changed?(bar_values)
            if bar_values.include? nil
              bar_values.each_with_index { |value, index| @driver.write_to_bar(index, value) unless value.nil? }
            else
              @driver.write(bar_values)
            end
          end
        end
      end

      state :display_bar_animation do
        def write
          raise NotImplementedError
        end
      end
    end

    def values_changed?(bar_values)
      if @current_state_parameters[:last_values].nil?
        true
      else
        bar_values.each_with_index do |value, index|
          unless bar_values[index].nil?
            if @current_state_parameters[:last_values][index] != value
              return true
            end
          end
        end
      end
    end
  end

end