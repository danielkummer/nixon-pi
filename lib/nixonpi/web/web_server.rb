require 'sinatra'
require 'sinatra/base'
require 'sinatra/contrib'
require 'sinatra/activerecord'
require 'chronic_duration'
require 'haml'
require 'json'
require 'active_record'
require 'sinatra/form_helpers'
require 'sinatra/jsonp'

require_relative '../../../lib/nixonpi/command_queue'
require_relative '../command_parameters'
require_relative '../configurations/state_hash'
require_relative '../configurations/settings'
require_relative '../logging/logging'
require_relative 'models'

module NixonPi
  class WebServer < Sinatra::Base
    register Sinatra::ActiveRecordExtension
    helpers Sinatra::FormHelpers
    helpers Sinatra::Jsonp

    include Logging
    include CommandParameters
    extend Logging

    use Rack::MethodOverride

    set :database, 'sqlite:///db/settings.db'
    set :run, false
    set :root, File.dirname(__FILE__)
    set :public_folder, File.join(File.dirname(__FILE__), 'public')
    set :haml, {:format => :html5}
    set :port, Settings['web_server'].nil? ? '8080' : Settings['web_server']['port']

    at_exit do
      log.info "Sinatra shut down..., don't restart"
      exit
    end

    #error 400..510 do
    #  'Boom'
    #end

    not_found do
      'This is nowhere to be found.'
    end

    helpers do
      INDENT = '  ' # use 2 spaces for indentation
      def hash_to_haml(hash, level=0)
        result = ["#{INDENT * level}%ul"]
        hash.each do |key, value|
          if value.is_a?(Hash)
            result << "#{INDENT * (level + 1)}%li #{key}"
            result << hash_to_haml(value, level + 2)
          else
            result << "#{INDENT * (level + 1)}%li #{key}:#{value}"
          end
        end unless hash.nil?
        Haml::Engine.new(result.join("\n")).render
      end


      def format_command_values(data)
        #todo add .well div do list when rendering
        result = %w(%dl)
        [:state, :value, :animation_name, :options].each do |key|
          value = data.send(key)
          unless value.nil? and value != ""
            result << "  %dt #{key.to_s.gsub("_", " ").capitalize}"
            result << "  %dd #{value.to_s}"
          end
        end unless data.nil?
        Haml::Engine.new(result.join("\n")).render
      end
    end

    ###
    # GET REQUESTS
    ###

    ## Render the main control screen
    get '/' do
      @bar_count = Settings.in13_pins.size
      @lamp_count = Settings.in1_pins.size

      %w"tubes lamps bars".each do |state_machine|
        initial = Command.find(:first, conditions: ["initial = ? AND state_machine = ?", true, state_machine]) || Command.new(state_machine: state_machine)
        instance_variable_set("@#{state_machine}", initial)
      end

      haml :control, format: :html5
    end

    get '/scheduler.:format' do
      @schedules = Schedule.find(:all)
      haml :scheduler, format: :html5
    end

    #api currently unused
    get '/commands.:format' do
      commands = {}
      %w"tubes bars lamps power say".each do |type|
        commands[type] = command_parameters(type.to_sym)
      end
      formatted_response('json', commands, "Available commands")
    end

    get '/command/:target.:format' do
      cmd = params[:target]
      error 404 and return unless  %w"tubes bars lamps power say".include?(cmd)

      formatted_response('json', command_parameters(cmd.to_sym), "Options for command #{cmd}")
    end

    get '/info/:target.:format' do
      target = params[:target]

      error 400 and return unless %w"tubes bars lamps power".include?(target)

      if target == "power"
        data = NixonPi::PowerDriver.instance.get_params
      else
        data = NixonPi::HandlerStateMachine.get_params_for(target)
      end
      formatted_response(params[:format], data, "#{target} set to")
    end

    get '/info.:format' do
      data = {info: AbioCardClient.instance.info}
      formatted_response(params[:format], data, "Hardware information")
    end

    get '/logs.:format' do
      path = File.join(Dir.home, 'nixon-pi.log')
      @logs = `tail -n 1000 #{path}`.split("\n")

      format = params[:format]

      case format
        when 'json'
          halt jsonp(@logs)
        else
          haml :logs
      end

    end

    ###
    # POST REQUESTS
    ###

    post '/tubes' do
      preprocess_post_params(:tubes, @params) do |data|

        data[:value] = data[:value].to_s.rjust(12, " ") unless data[:value].nil?
        CommandQueue.enqueue(:tubes, data)
        save_data(data, :tubes)
        formatted_response('json', data, "Tubes set to")
      end
    end


    post '/lamps' do
      preprocess_post_params(:lamps, @params) do |data|
        result = Array.new(5) { 0 }

        data[:values].each do |v|
          result[v.to_i] = 1
        end unless data[:values].nil?

        data[:values] = result

        CommandQueue.enqueue(:lamps, data)
        save_data(data, :lamps)
        formatted_response('json', data, "Lamps set to")
      end
    end

    post '/bars' do
      preprocess_post_params(:bars, @params) do |data|
        CommandQueue.enqueue(:bars, data)
        save_data(data, :bars)
        formatted_response('json', data, "Bars set to")
      end
    end

    post '/scheduler' do
      preprocess_post_params(:scheduler, @params) do |data|
        #convert json to hash
        data[:command] = JSON.parse(data[:command])
        schedule = save_data(data, :schedule) #important! data must be saved before it's scheduled - because the id is used to auto delete records in the db
        data[:id] = schedule.id
        CommandQueue.enqueue(:schedule, data)

        formatted_response('json', data, "Bars set to")
      end
    end

    post '/say' do
      preprocess_post_params(:say, @params) do |data|
        CommandQueue.enqueue(:speech, data)
        formatted_response('json', data, "Speak ")
      end

    end

    post '/power', :provides => [:json] do
      preprocess_post_params(:power, @params) do |data|
        data[:value] = 0 if data.empty?
        CommandQueue.enqueue(:power, data)

        formatted_response('json', data, "Power set to")
      end
    end

    ###
    # DELETE REQUESTS
    ###

    delete '/schedule/:id' do |id|
      schedule = Schedule.find_by_id(id)
      data = Hash.new

      if !schedule
        data[:success] = false
      else
        schedule.destroy
        data[:success] = true
      end

      formatted_response('json', data, "Schedule deleted")
    end

    private
    ##
    # save to the sqlite3 database
    # @param [Hash] data attributes to save
    # @class_type [Object] Model class to save to; e.g Lamp, Bar, Tube
    def save_data(data, queue)
      prepare_db_data(data)
      ret = case queue
              when :schedule
                save_schedule(data)
              else
                save_command(data, queue) if data[:initial]
            end
    end

    def prepare_db_data(data)
      data.delete_if do |k, v| #delete explicitly unwanted entries
        %w"time cron-period cron-dom cron-month cron-mins cron-dow cron-time-hour cron-time-min splat captures".include?(k.to_s)
      end
    end

    def save_command(cmd, queue)
      data = cmd.clone

      data[:value] = data.delete(:values).join(",") if data[:values] #re-arrange values array to value string
      if data[:initial] #there's ever only one initial state per state machine - replace existing record
        command = Command.find(:first, conditions: ["initial = ? AND state_machine = ?", data[:initial], queue.to_s])
      end
      command ||= Command.new(state_machine: queue.to_s)
      log.error("Unable to update command attributes: #{data.to_s}") unless command.update_attributes(data)
      command
    end

    def save_schedule(data)
      schedule = Schedule.new
      log.error("Unable to update schedule attributes: #{data.to_s}") unless schedule.update_attributes(data)
      schedule
    end

    def preprocess_post_params(target, params)
      data = string_key_to_sym(params)
      data[:state_machine] = target.to_sym


      if target.to_sym == :schedule
        command = Schedule.new(data)
      else
        #this is hacky... refactor
        if data[:state].to_sym == :time
          data[:value] = data[:time_format]
        end
        data.delete(:time_format)
        command = Command.new(data)
      end

      command.valid?

      if command.errors.empty?
        yield data if block_given?
      else
        data[:success] = false
        data[:message] = ["ERROR for #{target.to_s.upcase}"]
        command.errors.each do |error, message|
          data[:message] << "#{error}: #{message}"
        end
        status 400
        formatted_response('json', data)
      end

    end

    def string_key_to_sym(hash)
      result = Hash.new
      hash.each { |k, v| result[k.to_sym] = v }
      result
    end

    def formatted_response(format, data, respond_message = "")
      data[:message] ||= []
      data[:success] = true unless data.has_key?(:success)
      data[:message] << respond_message unless respond_message.empty?

      case format.to_sym
        when :json
          data.delete(:time)
          halt jsonp(data)
        when :html
          halt haml(:formatted_response, :locals => data)
        else

      end
      error 406
      data[:success] = false
      data[:message] << "Unknown format"
      halt jsonp(data)
    end

  end
end