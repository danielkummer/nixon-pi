require 'sinatra'
require 'sinatra/base'
require 'sinatra/contrib'
require 'sinatra/activerecord'
require 'chronic_duration'
require 'haml'
require 'json'
require 'active_record'
require 'sinatra/form_helpers'


require_relative '../../../lib/nixonpi/command_queue'
require_relative '../configurations/state_hash'
require_relative '../configurations/settings'
require_relative '../logging/logging'

require_relative 'models'


module NixonPi
  class WebServer < Sinatra::Base
    register Sinatra::RespondWith
    register Sinatra::ActiveRecordExtension
    helpers Sinatra::FormHelpers


    extend Logging


    set :database, 'sqlite:///db/settings.db'


    at_exit do
      log.info "Sinatra shut down..., don't restart"
      exit
    end


    set :run, false
    set :json_encoder, JSON
    set :root, File.dirname(__FILE__)
    set :public_folder, File.join(File.dirname(__FILE__), 'public')
    set :haml, {:format => :html5}
    set :port, Settings['web_server'].nil? ? '8080' : Settings['web_server']['port']

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
        end
        Haml::Engine.new(result.join("\n")).render
      end
    end


    get '/' do
      @bar_count = Settings.in13_pins.size
      @lamp_count = Settings.in1_pins.size

      [:tubes, :lamps, :bars].each do |param|
        command = Command.find(:first, conditions: ["initial = ? AND state_machine = ?", false, param]) || Command.new(state_machine: param)
        instance_variable_set(("@" + param).to_sym, command)
      end

      haml :control, format: :html5
    end

    get '/info/?:state_machine?', :provides => [:html, :json] do

      if %w(tubes bars lamps).contains? params[:state_machine]

        state_machine = params[:state_machine]

        @info = NixonPi::HandlerStateMachine.state_parameters_for(state_machine)
        respond_with :state_info do |f|
          f.json { @info.to_json }
        end
      else
        status 400
        render '/' if content_type == :html
      end


    end

    get '/info', :provides => [:html, :json] do
      client = AbioCardClient.instance
      @info = client.info
      haml :info, locals: {info: @info}
    end

    ##
    # save to the sqlite3 database
    # @param [Hash] data attributes to save
    # @class_type [Object] Model class to save to; e.g Lamp, Bar, Tube
    def self.save_to_database(data, state_machine)
      if data[:initial]
        command = Command.find(:first, conditions: ["initial = ?", data[:initial], "state_machine = ?", state_machine.to_s])
      elsif data[:id]
        command = Command.find(:first, conditions: ["id = ?", data[:id, "state_machine = ?", state_machine.to_s]])
      else
        command = state_machine.new(state_machine: state_machine.to_s)
      end

      [:value, :animation_name, :options, :initial].each do |attr|
        command.send(attr, data[attr])
      end
      command.save
    end

    def self.valid_or_redirect(params)
      data = string_key_to_sym(params)
      if data.nil? or !data.has_key?(:mode) then
        status 400
        redirect("/")
      else
        yield if block_given?
      end
    end

    post '/tubes/?:id?' do
      valid_or_redirect(params) do
        case data[:mode].to_sym
          when :countdown
            chrono_format = ChronicDuration.parse(data[:value], format: :chrono)
            chrono_format.gsub!(/:/, ' ') #todo maybe not even space
          else
        end
        data[:value] = data[:value].rjust(12, " ") unless data[:value].nil?
        CommandQueue.enqueue(:tubes, data)
        save_to_database(data, Tube.class)
        status 200
        redirect '/' + params.to_param
      end
    end



    post '/say' do
      data = string_key_to_sym(params)
      CommandQueue.enqueue(:say, data)
    end

    def string_key_to_sym(hash)
      ret = {}
      hash.each do |k, v|
        ret[k.to_sym] = v
      end
      ret
    end
=begin
Examples with activerecord
get '/todo/create/?' do
   #This is very important/cool
   #Create a new object (but not sent to database)
   #Through the Activerecord (ORM) functions it will be initialised with the database defaults

   #The @new object can be used to determin whether the template is POST (Create) or PUT (Modify)
   @todo = Todo.new
   @new = true
   erb :'todo/todo_edit'
end

post '/todo/?' do
   @todo = Todo.create(
      :done => params['post']['done'],
      :desc => params['post']['desc']
   )
   #Retun to view of newly created item
   redirect '/todo/' + @todo.id.to_s
end

get '/todo/:id/edit/?' do
   @todo = Todo.find(:first, :conditions => ["id = ?", params[:id] ])
   erb :'todo/todo_edit'
end

put '/todo/:id/?' do
   @todo = Todo.find(:first, :conditions => ["id = ?", params[:id] ])
   @todo.done = params['post']['done']
   @todo.desc = params['post']['desc']
   @todo.save
   redirect '/todo/' + params[:id]
end

get '/todo/:id/?' do
   @todo = Todo.find(:first, :conditions => ["id = ?", params[:id] ])
   erb :'todo/todo_one'
end

=end

  end
end
