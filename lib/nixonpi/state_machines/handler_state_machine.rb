require 'state_machine'
require 'active_support/inflector'

require_relative '../logging/logging'
require_relative '../configurations/state_hash'
require_relative '../factory'
require_relative '../command_parameters'

require_relative '../web/models'
require 'active_record'


module NixonPi
  class HandlerStateMachine
    include Logging
    include Factory
    extend CommandParameters
    include CommandParameters

    @@state_parameters = {}

    def initialize
      super # NOTE: This *must* be called, otherwise states won't get initialized
    end

    ##
    # Get the state parameters for the registered state machine
    # @param [Symbol] type
    def self.state_parameters_for(type)
      @@state_parameters[type.to_sym]
    end

    ##
    # Get the current state information hash
    def state_info_hash
      current_state_parameters
    end

    ##
    # Main handle method for all state machines
    def handle
      write
    end

    ##
    # Get the current state parameters from the global class state hash, lazy initialized
    def current_state_parameters
      @@state_parameters[registered_as_type] ||= initialize_state
    end


    protected
    def reload_from_db(state_machine)
      ActiveRecord::Base.establish_connection("sqlite3:///db/settings.db")
      options = Command.find(:first, conditions: ["initial = ? AND state_machine = ?", true, state_machine])
      ActiveRecord::Base.connection.close
      if options
        log.debug "db setting loaded for #{state_machine} => #{options.to_s} "
        set_params(options)
        log.debug "state set to: #{current_state_parameters.to_s}"
      else
        log.debug "no db settings found for :#{state_machine} "
      end
    end

    def set_params(options)
      options.each do |option, value|
        current_state_parameters[option.to_sym] = value if current_state_parameters.has_key?(option.to_sym) and !value.nil?
      end
    end

    ##
    # Lazy initialize state hash if not already existing
    def initialize_state
      @@state_parameters[registered_as_type] = StateHash.new
      @@state_parameters[registered_as_type].merge(command_parameters(registered_as_type))
      @@state_parameters[registered_as_type]
    end

    ##
    # Register a driver to be used by the state machine
    def register_driver(driver)
      @driver = driver
    end

    ##
    # Handle around transitions, remember states
    # @param [Object] object State machine class instance
    # @param [Transition] transition
    # @param [block] block
    def self.handle_around_transition(object, transition, block)
      object.log.debug "transition  #{transition.event} from state: #{object.state}"
      object.current_state_parameters[:last_state] = object.state if !object.state.nil?
      #transition.event.to_s.humanize.to_speech #say the current state transition
      block.call
      object.current_state_parameters[:state] = object.state
      object.log.debug "new state: #{object.state}"
    end


    ##
    # Get an instance of the underlying driver
    def driver
      @driver.instance
    rescue "Driver missing"
    end

    ##
    # Receive command parameters to change the state of the current state machine
    #todo: this currently doesn't abords running animations
    # @param [Hash] command command parameters
    def receive(command)
      log.debug "received command: #{command.to_s} in #{self.class.to_s}"
      current_state_parameters.merge!(command)
      self.fire_state_event(command[:state].to_sym) if command[:state]
    end

    ##
    # detect if values changed in the bar values array
    #param [Array] Integer array of bar values
    #todo refactor
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
