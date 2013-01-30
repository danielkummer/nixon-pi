require 'state_machine'
require 'active_support/inflector'

require_relative '../logging/logging'
require_relative '../configurations/state_hash'
require_relative '../factory'
require_relative '../messaging/command_listener'
require_relative '../../../lib/nixonpi/information/information_holder'

require_relative '../../../web/models'
require 'active_record'


module NixonPi
  class HandlerStateMachine
    include Logging
    include Factory
    include CommandListener
    include InformationHolder

    attr_accessor :registered_as_type

    @state_parameters = {}

    def initialize()
      super() # NOTE: This *must* be called, otherwise states won't get initialized
      reload_state()
    end

    def reload_state
      ActiveRecord::Base.establish_connection("sqlite3:///db/settings.db")
      options = Command.find(:first, conditions: ["state_machine = ?", self.registered_as_type])
      ActiveRecord::Base.connection.close
      if options
        log.debug "db setting loaded for #{self.registered_as_type} => #{options.to_s} "
        self.set_params(options)
      else
        log.debug "no db settings found for :#{self.registered_as_type} "
      end
    end

    ##
    # Main handle method for all state machines
    # This method gets called in the main loop
    def handle
      write
    end

    ##
    # Receive command parameters to change the state of the current state machine
    #todo: this currently doesn't abord running animations
    # @param [Hash] command command parameters
    def handle_command(command)
      log.debug "got #{self.class.to_s} command: #{command.to_s}"
      params.merge!(command)
      if command[:state] and command[:state] != params[:last_state]
        self.fire_state_event(command[:state].to_sym)
      end
    end

    def handle_info_request(about)
      ret = Hash.new
      case about.to_sym
        when :params
          ret = get_params.clone
        when :commands
          ret = {commands: self.class.available_commands.clone}
        else
          log.error "No information about #{about}"
      end
      ret
    end


    ##
    # Get the state parameters for the registered state machine
    def get_params
      @state_parameters
    end


    ##
    # Get the current state parameters from the global class state hash, lazy initialized
    def params
      @state_parameters ||= initialize_state
    end


    protected
    #todo refactor!!!
    def reload_from_db(state_machine)
=begin
      ActiveRecord::Base.establish_connection("sqlite3:///db/settings.db")
      options = Command.find(:first, conditions: ["state_machine = ?", state_machine])
      ActiveRecord::Base.connection.close
      if options
        log.debug "db setting loaded for #{state_machine} => #{options.to_s} "
        set_params(options)
        log.debug "state set to: #{params.to_s}"
      else
        log.debug "no db settings found for :#{state_machine} "
      end
=end
    end

    def set_params(options)
      options.each do |option, value|
        params[option.to_sym] = value if params.has_key?(option.to_sym) and !value.nil?
      end
    end

    ##
    # Lazy initialize state hash if not already existing
    def initialize_state
      @state_parameters = StateHash.new

      self.class.available_commands.each do |cmd|
        @state_parameters[cmd] = nil
      end

      @state_parameters
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
      object.params[:last_state] = object.state if !object.state.nil?
      block.call
      object.params[:state] = object.state
      object.log.debug "TRANSITION:  #{object.params[:last_state]} --#{transition.event}--> #{object.state}"
      object.log.debug "new state: #{object.state}"
    end

    ##
    # Get an instance of the underlying driver
    def driver
      @driver.instance
    rescue
      log.error "Driver missing"
    end


  end
end
