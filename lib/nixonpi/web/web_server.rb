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

      def read_latest_logs
        path = File.join(Dir.home, 'nixon-pi.log')
        logs = `tail -n 1000 #{path}`.split("\n")
        logs
      end

      def format_loaded_values(data)
        result = ["%dl"]
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

    ##
    # save to the sqlite3 database
    # @param [Hash] data attributes to save
    # @class_type [Object] Model class to save to; e.g Lamp, Bar, Tube
    def save_data(data, queue)
      sanitize_data(data)
      if queue == :schedule
        ret = save_schedule(data)
      else
        ret = save_command(data, queue)
      end
      ret
    end

    def sanitize_data(data)
      data[:value] = data.delete(:values).join(",") if data[:values]
      data.delete_if do |k, v|
        %w"time cron-period cron-dom cron-month cron-mins cron-dow cron-time-hour cron-time-min splat captures".include?(k.to_s)
      end
    end

    def save_command(data, queue)
      #there's ever only one initial state per state machine
      if data[:initial]
        command = Command.find(:first, conditions: ["initial = ? AND state_machine = ?", data[:initial], queue.to_s])
        command = Command.new(state_machine: queue.to_s) if command.nil?
      elsif data[:id]
        command = Command.find(:first, conditions: ["id = ? AND state_machine = ?", data[:id], queue.to_s])
      else
        command = Command.new(state_machine: queue.to_s)
      end

      unless command.update_attributes(data)
        log.error("Unable to update command attributes: #{data.to_s}")
      end

      command
    end

    def save_schedule(data)
      schedule = Schedule.new
      unless schedule.update_attributes(data)
        log.error("Unable to update schedule attributes: #{data.to_s}")
      end
      schedule
    end


    def convert_and_validate(params)
      data = string_key_to_sym(params)
      if data.nil?
        status 400
      else
        #data[:command] = string_key_to_sym(data[:command]) if data.has_key?(:command)
        yield data if  block_given?
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
      data[:message] = respond_message unless respond_message.empty?
      data[:success] = true unless data.has_key?(:success)
      case format
        when 'json'
          data.delete(:time)
          halt jsonp(data)
        when 'html'
          halt haml(template, :locals => data) unless template.nil?
      end
      error 406
      halt jsonp(data)
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
      @schedules = Schedule.find(:all)
      haml :scheduler, format: :html5
    end

    #api currently unused
    get '/commands.:format' do
      commands = {}
      %w"tubes bars lamps power say".each do |type|
        commands[type] = command_parameters(type.to_sym)
      end

      custom_respond('json', commands, "Available commands")
    end

    get '/command/:name.:format' do
      valid_commands = %w"tubes bars lamps power say"
      type = params[:name]
      error 404 and return unless valid_commands.include?(type)
      custom_respond('json', command_parameters(type.to_sym), "Options for command #{type} ")
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
        save_data(data, :tubes)


        custom_respond('json', data, "Tubes set to")
      end
    end

    post '/lamps/?:id?' do
      convert_and_validate(@params) do |data|
        CommandQueue.enqueue(:lamps, data)
        save_data(data, :lamps)
        custom_respond('json', data, "Lamps set to")
      end
    end

    post '/bars/?:id?' do

      convert_and_validate(@params) do |data|
        CommandQueue.enqueue(:bars, data)
        save_data(data, :bars)
        custom_respond('json', data, "Bars set to")
      end
    end

    post '/scheduler' do
      convert_and_validate(@params) do |data|
        if valid_schedule?(data)
          #convert json to hash
          data[:command] = JSON.parse(data[:command])
          schedule = save_data(data, :schedule) #important! data must be saved before it's scheduled - because the id is used to auto delete records in the db
          data[:id] = schedule.id
          CommandQueue.enqueue(:schedule, data)
        else
          log.error "Schedule invalid: #{data}"
        end

        custom_respond('json', data, "Bars set to")
      end
    end

    delete '/schedule/:id' do |id|
      schedule = Schedule.find_by_id(id)

      data = Hash.new

      if !schedule
        data[:success] = false
      else
        schedule.destroy
        data[:success] = true
      end

      custom_respond('json', data, "Schedule deleted")
    end


    def valid_schedule?(data)
      valid = true
      require 'rufus-scheduler'

      if %w"in at".include?(data[:timing])
        begin
          time = Rufus.parse_time_string data[:time]
        rescue ArgumentError => e
          log.error("Schedule invalid: #{e.message}")
          valid = false
        end
      end

      valid
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

    #run directly on app-file exec
    #run! if app_file == $0

  end
end