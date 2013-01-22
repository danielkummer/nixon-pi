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
require_relative '../configurations/state_hash'
require_relative '../configurations/settings'
require_relative '../logging/logging'
require_relative 'models'
require_relative '../../blank_monkeypatch'

module NixonPi
  class WebServer < Sinatra::Base
    register Sinatra::ActiveRecordExtension
    helpers Sinatra::FormHelpers
    helpers Sinatra::Jsonp

    include Logging
    extend Logging

    use Rack::MethodOverride

    set :database, 'sqlite:///db/settings.db'
    set :run, false
    set :app_file, __FILE__
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


    get '/' do
      @no_of_bars = Settings.in13_pins.size
      @no_of_lamps = Settings.in1_pins.size
      %w(tubes lamps bars).each do |state_machine|
        initial = Command.find(:first, conditions: ["state_machine = ?", state_machine]) || Command.new(state_machine: state_machine)
        instance_variable_set("@#{state_machine}", initial)
      end
      haml :control, format: :html5
    end

    get '/scheduler.?:format?' do
      @schedules = Schedule.find(:all)
      haml :scheduler, format: :html5
    end


    #todo completely deprecated - can't be done anymore because no central command register
    #could be done using the commandprocessor and iterating over every receiver...
    get '/commands.:format?' do
      commands = {}
      %w(tubes bars lamps power say).each do |type|
        commands[type] = command_parameters(type.to_sym)
      end
      formatted_response('json', commands, "Available commands")
    end

    get '/command/:target.:format?' do
      target = params[:target]

      begin
        receiver = NixonPi::CommandProcessor.get_receiver_for(target)
        data = receiver.available_commands

      rescue Exception => e
        data = {
            message: ["Options for #{target} not found"],
            success: false
        }
      end

      formatted_response('json', data, "Options for command #{target}")
    end


    get '/info/:target/?.:format?' do
      target = params[:target]

      unless %w(tubes bars lamps power).include?(target)
        halt 400
      end

      data = Hash.new

      case target.to_sym
        when :power
          data = NixonPi::PowerDriver.instance.get_params
        when :bars
          data[:bars] = Array.new
          %w(bar0 bar1 bar2 bar3).each do |bar|
            data[:bars] << NixonPi::MachineManager.get_params_for(bar)
          end
        when :lamps
          data[:lamps] = Array.new
          %w(lamp0 lamp1 lamp2 lamp3 lamp4).each do |lamp|
            data[:lamps] << NixonPi::MachineManager.get_params_for(lamp)
          end
        else
          data = NixonPi::MachineManager.get_params_for(target.to_sym)
      end

      formatted_response(params[:format], data, "#{target} state")

    end

    #todo refactor
    get '/info/:target/:id.:format?' do
      target = params[:target]
      id = params[:id]

      target = "#{target}#{id}"
      data = NixonPi::MachineManager.get_params_for(target)

      if data.nil?
        data = {
            message: ["Information for #{target} not found"],
            success: false
        }
      end

      formatted_response(params[:format], data, "#{target} set to")
    end


    get '/info/hw/?.:format?' do
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

    post '/tubes/?' do
      preprocess_post_params(:tubes, @params) do |data|
        CommandQueue.enqueue(:tubes, data)
        formatted_response('json', data, "Tubes set to")
      end
    end


    post '/lamp/?' do
      id = params[:id]
      preprocess_post_params(:lamp, @params) do |data|
        CommandQueue.enqueue("lamp#{id}".to_sym, data)
        formatted_response('json', data, "Lamps set to")
      end
    end

    post '/bar/?' do
      #todo error when no id
      id = params[:id]
      preprocess_post_params(:bars, @params) do |data|
        CommandQueue.enqueue("bar#{id}".to_sym, data)
        formatted_response('json', data, "Bar #{id} set to")
      end
    end

    post '/scheduler/?' do
      preprocess_post_params(:scheduler, @params) do |data|
        #convert json to hash
        data[:command] = JSON.parse(data[:command])
        data[:id] = schedule.id
        CommandQueue.enqueue(:schedule, data)

        formatted_response('json', data, "Bars set to")
      end
    end

    post '/say/?' do
      preprocess_post_params(:speech, @params) do |data|
        CommandQueue.enqueue(:speech, data)
        formatted_response('json', data, "Speak ")
      end

    end

    post '/power/?', :provides => [:json] do
      @params[:value] = 0 if @params.empty?

      preprocess_post_params(:power, @params) do |data|
        CommandQueue.enqueue(:power, data)
        formatted_response('json', data, "Power set to")
      end
    end

    ###
    # DELETE REQUESTS
    ###

    delete '/schedule/:id/?' do |id|
      schedule = Schedule.find_by_id(id)
      data = Hash.new

      if !schedule
        data[:success] = false
        data[:message] = ["Schedule not found"]
      else
        schedule.destroy
        data[:success] = true
        data[:message] = ["Schedule deleted"]
      end

      formatted_response('json', data)
    end

    private

    #todo this badly needs refactoring
    def preprocess_post_params(target, params)
      data = string_key_to_sym(params)
      data[:state_machine] = target.to_sym

      #this is hacky... refactor
      case target.to_sym
        when :schedule
          command = Schedule.new(data)
        when :tubes
          data[:value] = data[:value].to_s.rjust(12, " ") unless data[:value].nil?
          if data[:state].to_sym == :time
            data[:value] = data[:time_format]
          end
          data.delete(:time_format)
        else
          data.delete(:id) #not intended for ar - use

      end
      command ||= Command.new(data)

      command.valid?

      if command.errors.empty?
        #save if necessary, todo refactor
        if target == :schedule
          command.save
        elsif params[:initial]
          initial = Command.find(:first, conditions: ["state_machine = ?", target.to_s])
          initial ||= command
          log.error("Unable to update command attributes: #{data.to_s}") unless initial.update_attributes(data)
        end

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