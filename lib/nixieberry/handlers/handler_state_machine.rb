require 'state_machine'

require_relative '../logging/logging'
require_relative '../configurations/state_hash'
require 'active_support/inflector'
require_relative '../command_queue'
require_relative '../control_parameters'
require_relative '../factory'


module NixieBerry
  class HandlerStateMachine
    include Logging
    include CommandQueue
    include Factory
    extend ControlParameters

    @@state_parameters = {}

    def initialize
      super() # NOTE: This *must* be called, otherwise states won't get initialized
    end

    def self.state_parameters_for(type)
      @@state_parameters[type]
    end

    ##
    # Get the current state parameters from the global class state hash, lazy initialized
    def current_state_parameters
      @@state_parameters[registered_as_type] ||= initialize_state_hash
    end

    def state_information
      current_state_parameters
    end

    def handle
      handle_command_queue
      write
    end

    protected
    ##
    # Lazy initialize state hash if not already existing
    def initialize_state_hash
      @@state_parameters[registered_as_type] = StateHash.new
      @@state_parameters[registered_as_type][:last_state] = nil
      @@state_parameters[registered_as_type]
    end


    ##
    # Register a driver to be used by the state machine
    def register_driver(driver)
      @driver = driver
    end

    ##
    # Get an instance of the underlying driver
    def driver
      @driver.instance
    rescue "Driver missing"
    end

    ##
    # Handle the queue assigned to the registered type, assign state parameters and do the state switch
    def handle_command_queue
      unless queue(registered_as_type).empty?
        state_change = queue(registered_as_type).pop
        log.debug("New possible state change: #{state_change}")
        state_change.delete_if { |k, v| !control_parameters(registered_as_type).keys.include?(k) or v.nil? }

        #do nothing if command is older than 2 seconds
        if state_change[:time] + 2 > Time.now
          log.debug("State change accepted: #{state_change}")
          current_state_parameters.merge!(state_change)
          #trigger the event
          self.fire_state_event(state_change[:mode].to_sym) if state_change[:mode] and self.state != @@state_parameters[registered_as_type][:last_state]
        end
      end
    end

    def self.handle_around_transition(object, transition, block)
      object.log.debug "transition  #{transition.event} from state: #{object.state}"
      object.current_state_parameters[:last_state] = object.state
      #transition.event.to_s.humanize.to_speech #say the current state transition
      block.call
      object.current_state_parameters[:state] = object.state
      object.log.debug "new state: #{object.state}"
    end

    def values_changed?(bar_values)
      if current_state_parameters[:last_values].nil?
        true
      else
        bar_values.each_with_index do |value, index|
          unless bar_values[index].nil?
            if current_state_parameters[:last_values][index] != value
              return true
            end
          end
        end
      end
    end
  end
end
