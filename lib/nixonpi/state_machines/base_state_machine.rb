require 'state_machine'
require 'active_support/inflector'

require_relative '../logging/logging'
require_relative '../configurations/state_hash'
require_relative '../factory'
require_relative '../messaging/commands_module'
require_relative '../../../lib/nixonpi/information/information_holder'
require_relative '../messaging/command_receiver'
require_relative '../messaging/command_sender'

require_relative '../../../web/models'
require_relative '../../dependency'
require 'active_record'


module NixonPi
  class BaseStateMachine
    include Logging

    include Commands
    include InformationHolder

    attr_accessor :registered_as_type

    @state_parameters = {}

    def initialize()
      super() # NOTE: This *must* be called, otherwise states won't get initialized
              #reload_state()
    end

    state_machine :initial => :startup do

      around_transition do |object, transition, block|
        BaseStateMachine.handle_around_transition(object, transition, block)
      end

      event(:free_value) { transition all => :free_value }
      event(:animation) { transition all => :animation }
      event(:free_value) { transition all => :free_value }
      event(:animation) { transition all => :animation }


      state :animation do
        def enter_state
          name, options = params[:animation_name], params[:options]
          options ||= {}
          if params[:value] and !params[:value].strip.empty?
            options["start_value"] = params[:value]
          end
          handle_command(state: params[:last_state]) if options.empty? #leave state if options are empty!!
          @animation = get_injected(name.to_sym, true, options)
          @animation.use_driver(@driver)
        end

        def leave_state
          #@animation = nil
        end

        def write
          raise "Animation can't be empty!! in #{self.class.name}" if @animation.nil?
          @animation.write unless @animation.nil?
        end
      end

    end

    def reload_state
      ActiveRecord::Base.establish_connection("sqlite3:///db/settings.db")
      options = Command.find(:first, conditions: ["target LIKE ?", self.registered_as_type])
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
      #todo add new param -> after state to specify a transition to go to  - can't use last state because it's aready used... - maybe its not needed anymore
      log.debug "got #{self.registered_as_type.to_s} command: #{command.to_s}"
      current_state = params[:state]
      params.merge!(command)
      if command[:state]
        self.fire_state_event(command[:state].to_sym) if command[:state] != current_state
      end
    end

    def handle_info_request(about)
      result = Hash.new
      case about.to_sym
        when :params
          begin
            result = Hash.new
            @state_parameters.each do |k,v|
              result[k] = v
            end
            result
          rescue
            log.error "error: unable to dump state parameters!"
          end
        when :commands
          commands = Marshal.load(Marshal.dump(self.class.available_commands))
          result[:commands] = commands
        else
          log.error "No information about #{about}"
      end
      result
    end

    ##
    # Get the current state parameters from the global class state hash, lazy initialized
    def params
      @state_parameters ||= initialize_state
    end


    protected
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
      object.params[:goto_state] = object.state if object.params[:goto_state].nil? and !object.state.nil?
      object.params[:last_state] = object.state
      begin
        object.leave_state
      rescue NoMethodError => e;
      end
      block.call
      #NixonPi::Messaging::CommandSender.new.send_command(:sound, {value: "Entering  #{object.state} state for #{object.registered_as_type}"}) unless object.params[:last_state] == "startup"
      object.params[:state] = object.state
      begin
        object.try(:enter_state)
      rescue NoMethodError => e;
        #log.info "Machine #{object.to_s} with state #{object.state} doesn't have the enter_state method"
      end
    end

  end
end
