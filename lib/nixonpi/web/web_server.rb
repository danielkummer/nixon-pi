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

module NixonPi
  class WebServer < Sinatra::Base
    register Sinatra::ActiveRecordExtension
    helpers Sinatra::FormHelpers
    helpers Sinatra::Jsonp

    extend Logging

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

      def read_latest_logs
        path = File.join(Dir.home, 'nixon-pi.log')
        logs = `tail -n 1000 #{path}`.split("\n")
        logs
      end

      def format_loaded_values(data)
        result = ["%dl"]
        [:state_machine, :state, :value, :animation_name, :options].each do |key|
          value = data.send(key)
          unless value.nil? and value != ""
            result << "  %dt #{key.to_s.gsub("_", " ").capitalize}"
            result << "  %dd #{value.to_s}"
          end
        end unless data.nil?

        Haml::Engine.new(result.join("\n")).render
      end
    end

    ##
    # save to the sqlite3 database
    # @param [Hash] data attributes to save
    # @class_type [Object] Model class to save to; e.g Lamp, Bar, Tube
    def save_to_database(data, state_machine)
      command = nil
      #todo refactor
      if data[:initial]
        command = Command.find(:first, conditions: ["initial = ? AND state_machine = ?", data[:initial], state_machine.to_s])
      elsif data[:id]
        command = Command.find(:first, conditions: ["id = ? AND state_machine = ?", data[:id], state_machine.to_s])
      else
        command = Command.new(state_machine: state_machine.to_s)
      end
      #todo error here! command is nil
      command = Command.new(state_machine: state_machine.to_s) if command.nil?

      data[:value] = data.delete(:values).join(",") if data[:values]

      unless command.update_attributes(data)
        log.debug("unable to update attributes: #{data.to_s}")
      end
    end


    def convert_and_validate(params)
      data = string_key_to_sym(params)
      if data.nil?
        status 400
      else
        yield data if block_given?
      end
    end

    def string_key_to_sym(hash)
      ret = {}
      hash.each do |k, v|
        ret[k.to_sym] = v
      end
      ret
    end

    def custom_respond(format, data, respond_message = "", template = nil)
      data[:message] = respond_message
      case format
        when 'json'
          halt jsonp(data)
        when 'html'
          halt haml(template, :locals => data) unless template.nil?
      end
      error 406
    end


    get '/' do
      @bar_count = Settings.in13_pins.size
      @lamp_count = Settings.in1_pins.size

      [:tubes, :lamps, :bars].each do |param|
        command = Command.find(:first, conditions: ["initial = ? AND state_machine = ?", false, param]) || Command.new(state_machine: param)
        instance_variable_set("@#{param}", command)
        intitial = Command.find(:first, conditions: ["initial = ? AND state_machine = ?", true, param]) || Command.new(state_machine: param)
        instance_variable_set("@#{param}_settings", intitial)
      end

      haml :control, format: :html5
    end

    get '/scheduler.:format' do
      haml :scheduler, format: :html5
    end

    get '/info/:state_machine.:format' do
      if %w(tubes bars lamps).include? params[:state_machine]
        state_machine = params[:state_machine]
        data = {info: NixonPi::HandlerStateMachine.get_params_for(state_machine)}
        custom_respond(params[:format], data, "#{state_machine} set to", :state_info)
      else
        error 400
      end

    end

    get '/info.:format' do
      data = {info: AbioCardClient.instance.info}
      custom_respond(params[:format], data, "Hardware information", :info)
    end

    get '/logs' do
      haml :logs
    end


    post '/tubes/?:id?' do
      convert_and_validate(@params) do |data|
        unless data[:state].nil?
          case data[:state].to_sym
            when :countdown
              data[:value] = ChronicDuration.parse(data[:value], format: :chrono)
            #chrono_format.gsub!(/:/, ' ') #todo maybe not even space
            else
          end
        end
        data[:value] = data[:value].to_s.rjust(12, " ") unless data[:value].nil?
        CommandQueue.enqueue(:tubes, data)
        save_to_database(data, :tubes)


        custom_respond('json', data, "Tubes set to")
      end
    end

    post '/lamps/?:id?' do
      convert_and_validate(@params) do |data|
        CommandQueue.enqueue(:lamps, data)
        save_to_database(data, :lamps)
        custom_respond('json', data, "Lamps set to")
      end
    end

    post '/bars/?:id?' do

      convert_and_validate(@params) do |data|
        CommandQueue.enqueue(:bars, data)
        save_to_database(data, :bars)
        custom_respond('json', data, "Bars set to")
      end
    end


    post '/say' do
      convert_and_validate(@params) do |data|
        CommandQueue.enqueue(:say, data)
        custom_respond('json', data, "Told ")
      end

    end

    post '/power', :provides => [:json] do
      convert_and_validate(@params) do |data|
        data[:value] = 0 if data.empty?
        CommandQueue.enqueue(:power, data)

        custom_respond('json', data, "Power set to")
      end
    end

    #todo
    post '/scheduler', :provides => [:json] do

      begin
        Rufus.parse_time_string params[:time] if params[:type] == 'in' or params[:type] == 'every'
      rescue ArgumentError => e
        log.error "time parsing error: #{e}"
      end

      #todo param validation and scheduling
      #Scheduler.schedule(type,time,target,command)
    end

    #run directly on app-file exec
    #run! if app_file == $0

  end
end