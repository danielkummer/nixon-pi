require 'sinatra'
require 'sinatra/base'
require 'sinatra/contrib'
require 'sinatra/activerecord'
require 'chronic_duration'
require 'haml'
require 'json'
require 'active_record'

require_relative '../../../lib/nixonpi/command_queue'
require_relative '../configurations/state_hash'
require_relative '../configurations/settings'
require_relative '../logging/logging'


module NixonPi
  class WebServer < Sinatra::Base
    register Sinatra::RespondWith
    extend Logging

    register Sinatra::ActiveRecordExtension


    at_exit do
      log.info "Sinatra shut down..., don't restart"
      exit
    end

    set :database, 'sqlite://../../../db/settings.db'


    #This is automagically linked with the plural table (todos)
    class Tube < ActiveRecord::Base
    end
    class Bar < ActiveRecord::Base
    end
    class Lamp < ActiveRecord::Base
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
      @bars = Settings.in13_pins
      @lamps = Settings.in1_pins

      #todo refactor
      #load default values if there are some..
      @lamp = Lamp.first || Lamp.new
      #@bar = Bar.first || Bar.new
      #@tube = Tube.first || Tube.new
      #todo use the settings in view

      @tube_data = NixonPi::HandlerStateMachine.state_parameters_for(:tubes)
      @bar_data = NixonPi::HandlerStateMachine.state_parameters_for(:bars)
      @lamp_data = NixonPi::HandlerStateMachine.state_parameters_for(:lamps)

      haml :control
    end

    get '/tubes', :provides => [:html, :json] do
      @info = NixonPi::HandlerStateMachine.state_parameters_for(:tubes)
      respond_with :state_info do |f|
        f.json { @info.to_json }
      end
    end

    get '/lamps', :provides => [:html, :json] do
      @info = NixonPi::HandlerStateMachine.state_parameters_for(:lamps)
      respond_with :state_info do |f|
        f.json { @info.to_json }
      end
    end

    get '/bars', :provides => [:html, :json] do
      @info = NixonPi::HandlerStateMachine.state_parameters_for(:bars)
      respond_with :state_info do |f|
        f.json { @info.to_json }
      end
    end

    get '/info', :provides => [:html, :json] do
      client = AbioCardClient.instance
      @info = client.info
      haml :info, locals: {info: @info}
    end

    post '/tubes/?' do
      data = string_key_to_sym(params)
      if data.nil? or !data.has_key?(:mode) then
        status 400
        redirect("/")
      else
        case data[:mode].to_sym
          when :countdown
            chrono_format = ChronicDuration.parse(data[:value], format: :chrono)
            chrono_format.gsub!(/:/, ' ') #todo maybe not even space
          else
        end
        data[:value] = data[:value].rjust(12, " ") unless data[:value].nil?


        CommandQueue.enqueue(:tubes, data)

        @lamp = Lamp.find_or_create(:first, :conditions => ["id = ?", params[:id]])
        @lamp.mode = data[:mode]
        @lamp.value = data[:value]
        @lamp.animation_name = data[:animation_name]
        @lamp.options = params[:options]
        @lamp.save
        status 200
        redirect '/todo/' + params[:id]
      end
    end

    post '/bars' do
      data = string_key_to_sym(params)
      if data.nil? or !data.has_key?(:mode) then
        status 400
        redirect ("/")
      else
        CommandQueue.enqueue(:bars, data)
        status 200
        redirect("/")
      end
    end

    post '/lamps' do
      data = string_key_to_sym(params)
      if data.nil? or !data.has_key?(:mode) then
        status 400
        redirect ("/")
      else
        CommandQueue.enqueue(:lamps, data)
        status 200
        redirect("/")
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
