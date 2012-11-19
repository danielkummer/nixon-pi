require 'state_machine'

require_relative '../logging/logging'
require_relative '../configurations/state_hash'
require 'active_support/inflector'
require_relative '../command_queue'
require_relative '../control_parameters'


module NixieBerry
  class HandlerStateMachine
    include Logging
    include CommandQueue
    extend ControlParameters

    StateMachine::Machine.ignore_method_conflicts = true

    attr_accessor :current_state_parameters

    def self.register_driver(driver)
      @driver = driver
    end

    def self.register_queue_name(name)
      @queue_name = name.to_sym
    end

    def initialize

      @current_state_parameters = StateHash.new
      @current_state_parameters[:last_state] = nil

      super() # NOTE: This *must* be called, otherwise states won't get initialized
    end


    def state_information
      @current_state_parameters
    end

    def handle
      handle_command_queue
      write
    end

    protected
    def handle_command_queue
      unless queue(@queue_name).empty?
        state_change = queu(@queue_name).pop
        log.debug("New possible state change: #{state_change}")
        state_change.delete_if { |k, v| !control_parameters(@queue_name).keys.include?(k) or v.nil? }

        #do nothing if command is older than 2 seconds
        if state_change[:time] + 2 > Time.now
          log.debug("State change accepted: #{state_change}")
          @current_state_parameters.merge(state_change)
          #trigger the event
          self.fire_state_event(state_change[:mode].to_sym) if state_change[:mode] and self.state != state_change[:mode]
        end
      end
    end

    def handle_around_transition(object, transition, block)
      object.log.debug "transition  #{transition.event} from state: #{object.state}"
      object.current_state_parameters[:last_state] = object.state
      #transition.event.to_s.humanize.to_speech #say the current state transition
      block.call
      object.current_state_parameters[:state] = object.state
      object.log.debug "new state: #{object.state}"
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
