require 'sinatra'
require 'sinatra/base'
require 'sinatra/contrib'
#require 'json'
require_relative '../configurations/state_hash'
require 'haml'
require_relative '../../../lib/nixieberry/command_queue'
require_relative '../configurations/settings'
require_relative '../logging/logging'
require 'json'

module NixieBerry
  class WebServer < Sinatra::Base
    register Sinatra::RespondWith
    include CommandQueue
    extend Logging

    at_exit do
      log.info "Sinatra shut down..., don't restart"
      exit
    end

    set :run, false
    set :server, "thin"
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
      haml :control
    end

    get '/tubes', :provides => [:html, :json] do
      @info = NixieBerry::HandlerStateMachine.state_parameters_for(:tubes)
      respond_with :tubes do |f|
        f.json { @info.to_json }
      end
    end
  end

  get '/info', :provides => [:html, :json] do
    client = AbioCardClient.instance
    @info = client.info
    haml :info, locals: {info: @info}
  end

  post '/tubes' do
    #data = JSON.parse(params[:data])
    data = sanitize(params)
    if data.nil? or !data.has_key?(:mode) then
      status 400
      redirect("/")
    else
      data[:value] = data[:value].rjust(12, " ")
      enqueue(:tubes, data)

      status 200
      redirect("/")
    end
  end

  post '/bars' do
    data = params
    if data.nil? or !data.has_key?(:mode) then
      status 400
      redirect ("/")
    else
      enqueue(:bars, data)
      status 200
      redirect("/")
    end
  end

  post '/lamps' do
    data = params
    if data.nil? or !data.has_key?(:mode) then
      status 400
      redirect ("/")
    else
      enqueue(:lamps, data)
      status 200
      redirect("/")
    end
  end

  post '/say' do
    data = params
    enqueue(:say, data)
  end

end
