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
require 'drb'
require_relative '../lib/version'

$environment = ENV['RACK_ENV']

require_relative '../lib/nixonpi/configurations/settings'
require_relative 'models'
require_relative '../lib/blank_monkeypatch'
require_relative '../lib/nixonpi/messaging/command_sender'
require_relative '../lib/nixonpi/hash_monkeypatch'

REMOTE_INFO_PROXY = DRbObject.new_with_uri('druby://localhost:9001')
DRb.start_service

module NixonPi
  class WebServer < Sinatra::Base
    set :root, File.dirname(__FILE__)
    register Sinatra::ActiveRecordExtension

    helpers Sinatra::FormHelpers
    helpers Sinatra::Jsonp

    use Rack::MethodOverride

    #todo always development
    set :environment => ENV['RACK_ENV'].to_sym
    set :database, 'sqlite:///../db/settings.db'
    #set :lock, false #enable on threading errors
    set :public_folder, File.join(File.dirname(__FILE__), 'public')
    set :haml, {:format => :html5}
    set :port, Settings['web_server'].nil? ? '8080' : Settings['web_server']['port']

    #set :static, $environment == 'development' ? false : true

    #set :show_exceptions, false
    disable :raise_errors
    disable :show_exceptions


    not_found do
      if request.accept? 'application/json'
        content_type :json
        status 404
        halt({:success => 'false', :message => "404 - are you sure it's there?"}.to_json)
      end

      haml 'errors/not_found'.to_sym, layout: false, locals: {info: {title: "404 - are you sure it's there?", message: "Nothing found!"}}
    end

    error Bunny::TCPConnectionFailed do
      if request.accept? 'application/json'
        content_type :json

        halt({:success => 'false', :message => "Make sure your RabbitMQ server is up and running..."}.to_json)
      end

      haml 'errors/rabbitmq_down'.to_sym, layout: false, locals: {info: {title: "Oops - I found a dead bunny...", message: "Make sure your RabbitMQ server is up and running..."}}
    end

    error do
      if request.accept? 'text/html'
        haml 'errors/rabbitmq_down'.to_sym, layout: false, locals: {info: {title: "Something just went terribly wrong...", message: "Here's a funny error:#{$!.message}"}}
      else
        content_type :json
        halt({:success => 'false', :message => e.message}.to_json)
      end

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

      def version
        NixonPi::VERSION.to_s
      end

      def sender
        @sender ||= NixonPi::Messaging::CommandSender.new
      end
    end

###
# GET REQUESTS
###
    get '/' do
      sender.client #trigger 500 error

      @no_of_bars = Settings.in13_pins.size
      @no_of_lamps = Settings.in1_pins.size
      %w(tubes bar0 bar1 bar2 bar3).each do |state_machine|
        initial = Command.find(:first, conditions: ["state_machine = ?", state_machine]) || Command.new(state_machine: state_machine)
        instance_variable_set("@#{state_machine}", initial)
      end
      haml :control, format: :html5
    end

    get '/scheduler.?:format?' do
      @schedules = Schedule.find(:all)
      haml :scheduler, format: :html5
    end

    get '/command/:target.:format?' do
      target = params[:target]
      data = get_data_for(target, :commands)
      formatted_response('json', data, "Options for command #{target}")
    end

    get '/targets.:format?' do
      #get all available targets for commands
      data = get_data_for(:commands, :targets)
      formatted_response('json', data, "available command targets")
    end

    get '/information/:target/?.:format?' do
      target = params[:target]
      what = target == 'hardware' ? :io_card : :params
      data = get_data_for(target, what)
      formatted_response(params[:format], data, "#{target} information")
    end

    get '/information/:target/:id.:format?' do
      target = "#{params[:target]}#{params[:id]}"
      data = get_data_for(target, :params)
      formatted_response(params[:format], data, "#{target} information")
    end

    get '/logs.:format' do
      path = File.join(Dir.home, 'nixon-pi.log')
      @logs = `tail -n 1000 #{path}`.split("\n")
      case params[:format]
        when 'json'
          halt jsonp(@logs)
        else
          haml :logs
      end
    end

    #todo refactor
    get '/state.:format' do
      data = Hash.new
      data[:rabbitmq] = sender.connected?
      data[:service] = get_data_for(:power, :params)[:value]
      formatted_response(params[:format], data, "application state")
    end

    ###
    # POST REQUESTS
    ###

    post '/tubes/?' do
      preprocess_post_params(:tubes, @params) do |data|
        sender.send_command(:tubes, data)
        formatted_response('json', data, "Tubes set to")
      end
    end

    post '/lamp/?' do
      id = params[:id]
      @params['value'] = "0" unless  @params['value']
      preprocess_post_params(:lamp, @params) do |data|
        sender.send_command("lamp#{id}".to_sym, data)
        formatted_response('json', data, "Lamps set to")
      end
    end

    post '/bar/?' do
      #todo error when no id
      id = params[:id]
      preprocess_post_params(:bars, @params) do |data|
        sender.send_command("bar#{id}".to_sym, data)
        formatted_response('json', data, "Bar #{id} set to")
      end
    end

    post '/scheduler/?' do
      preprocess_post_params(:scheduler, @params) do |data|
        #convert json to hash
        data[:command] = JSON.parse(data[:command])
        sender.send_command(:schedule, data)
        formatted_response('json', data, "Bars set to")
      end
    end

    post '/say/?' do
      preprocess_post_params(:sound, @params) do |data|
        sender.send_command(:sound, data)
        formatted_response('json', data, "Speak ")
      end
    end

    post '/power/?', :provides => [:json] do
      @params[:value] = 0 if @params.empty?
      preprocess_post_params(:power, @params) do |data|
        sender.send_command(:power, data)
        formatted_response('json', data, "Power set to")
      end
    end

    ###
    # DELETE REQUESTS
    ###

    delete '/schedule/:id/?' do |id|
      schedule = Schedule.find_by_id(id)
      data = Hash.new
      if schedule
        schedule.destroy
        set_message!(data, "Schedule deleted", false)
      else
        set_message!(data, "Schedule not fount", false)
      end
      formatted_response('json', data)
    end

    private

    ##
    # Create (and save) activerecord model to validate posted params
    # @param [Symbol] target
    # @param [Hash] params
    def preprocess_post_params(target, params)
      data = params.string_key_to_sym
      data[:state_machine] = target.to_sym

      record = get_or_create_record_for(data)

      if record.valid?
        record.save if params[:initial]
        data[:id] = record.id
        yield data if block_given?
      else
        set_message!(data, record.errors.map { |error, message| "#{error}: #{message}" }, false)
        status 400
        formatted_response('json', data)
      end

    end

    ##
    # Set a response message
    # @param [Hash] data
    # @param [String] message
    # @param [Boolean] success
    def set_message!(data, message, success)
      data[:success] = success
      message = [message] unless message.is_a? Array
      data[:message] = message
    end

    ##
    # Get a record for a target or create one if it doesn't exist
    # @param [Hash] data
    def get_or_create_record_for(data)
      case data[:state_machine].to_sym
        when :scheduler
          Schedule.new(data) #schedules are always newly created - you can only delete existing ones
        else
          get_or_create_command(data)
      end
    end

    ##
    # Get a command object or create one, also do some minor value adjustments
    # @param [Hash] data
    def get_or_create_command(data)
      case data[:state_machine].to_sym
        when :tubes
          data[:value] = data[:value].to_s.rjust(12, " ") unless data[:value].nil?
          if data[:state].to_sym == :time
            data[:value] = data[:time_format]
          end
          data.delete(:time_format)
        else
          data.delete(:id) #not intended for ar - use

      end
      command = nil
      if params[:initial]
        initial = Command.find(:first, conditions: ["state_machine = ?", data[:state_machine].to_s])
        command = initial if initial.update_attributes(data)
      end
      command ||= Command.new(data)
      command
    end

    ##
    # Return a formated response object with an optional response message
    # @param [Symbol] format json or html
    # @param [Hash] data
    # @param [String] respond_message
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
      set_message!(data, "Unknown format", false)
      halt jsonp(data)
    end

    ##
    # RPC Connection to service, get data from InformationProxy
    # @param [Symbol] target information target
    # @param [Symbol] about regested information identifier
    def get_data_for(target, about)
      data = Hash.new
      begin
        case target.to_sym
          when :power
            data = REMOTE_INFO_PROXY.get_info_from(:power, about)
          when :bars
            data[:bars] = Array.new
            %w(bar0 bar1 bar2 bar3).each do |bar|
              data[:bars] << REMOTE_INFO_PROXY.get_info_from(bar.to_sym, about)
            end
          when :lamps
            data[:lamps] = Array.new
            %w(lamp0 lamp1 lamp2 lamp3 lamp4).each do |lamp|
              data[:lamps] << REMOTE_INFO_PROXY.get_info_from(lamp.to_sym, about)
            end
          else
            data = REMOTE_INFO_PROXY.get_info_from(target.to_sym, about)
        end
      rescue Exception => e
        set_message!(data, "Options for #{target} not found , #{e.message}", false)
      end
      data
    end

    run! if app_file == $0
  end
end
