require 'state_machine'
require 'active_support/inflector'

require_relative '../logging/logging'
require_relative '../configurations/state_hash'
require_relative '../factory'
require_relative '../command_parameters'

require_relative '../web/models'
require 'active_record'


module NixonPi
  #noinspection ALL
  class HandlerStateMachine
    include Logging
    include Factory
    extend CommandParameters
    include CommandParameters

    attr_accessor :registered_as_type

    @@state_parameters = {}

    def initialize()
      super() # NOTE: This *must* be called, otherwise states won't get initialized
    end
    ##
    # Main handle method for all state machines
    # This method gets called in the main loop
    def handle
      write
    end

    ##
    # Get the state parameters for the registered state machine
    # @param [Symbol] type State machine
    def self.get_params_for(type)
      @@state_parameters[type.to_sym]
    end


    ##
    # Get the current state parameters from the global class state hash, lazy initialized
    def params
      @@state_parameters[registered_as_type] ||= initialize_state
    end


    protected
    #todo refactor!!!
    def reload_from_db(state_machine)
      ActiveRecord::Base.establish_connection("sqlite3:///db/settings.db")
      options = Command.find(:first, conditions: ["state_machine = ?", true, state_machine])
      ActiveRecord::Base.connection.close
      if options
        log.debug "db setting loaded for #{state_machine} => #{options.to_s} "
        set_params(options)
        log.debug "state set to: #{params.to_s}"
      else
        log.debug "no db settings found for :#{state_machine} "
      end
    end

    def set_params(options)
      options.each do |option, value|
        params[option.to_sym] = value if params.has_key?(option.to_sym) and !value.nil?
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
      object.params[:last_state] = object.state if !object.state.nil?
      #transition.event.to_s.humanize.to_speech #say the current state transition
      block.call
      object.params[:state] = object.state
      object.log.debug "new state: #{object.state}"
    end

    ##
    # Get an instance of the underlying driver
    def driver
      @driver.instance
    rescue
      log.error "Driver missing"
    end

    ##
    # Receive command parameters to change the state of the current state machine
    #todo: this currently doesn't abords running animations
    # @param [Hash] command command parameters
    #noinspection RubyResolve
    def receive(command)
      log.debug "received command: #{command.to_s} in #{self.class.to_s}"
      params.merge!(command)
      self.fire_state_event(command[:state].to_sym) if command[:state]
    end


  end
end
