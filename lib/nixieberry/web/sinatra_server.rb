require 'sinatra'
require 'json'
require_relative '../configurations/state_hash'
require 'haml'
require_relative '../../../lib/nixieberry/command_queue'
require_relative '../configurations/settings'
require_relative '../logging/logging'

module NixieBerry
  class RESTServer < Sinatra::Base
    include CommandQueue
    extend Logging

    at_exit { log.info "Sinatra shut down..., don't restart" }

    #set :static, true
    #set :run, true
    set :root, File.dirname(__FILE__)
    #set :public_folder, Proc.new { File.join(root, "public") }
    set :public_folder, File.join(File.dirname(__FILE__), 'public')


    #set :views, File.expand_path('../views', __FILE__) # set up the views dir
    set :haml, {:format => :html5} # if you use haml
    set :port, Settings['rest_server'].nil? ? '8080' : Settings['rest_server']['port']

    before '/\s' do
      content_type 'application/json'
    end

    get '/' do
      #body Control.instance.to_json
      haml :control
    end

    get '/tubes' do
      @info = NixieBerry::TubeHandlerStateMachine.instance.state_information
      #body({tubes: Control.instance[:tubes]}.to_json)
      content_type 'application/json'
      @info.to_json
    end

    get '/info' do
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

    private

    # @Deprecated
    def sanitize(data)
      data = Hash[data.map { |a| [a.first.to_sym, a.last] }] #to sym hash

      case data[:mode]
        when 'display_free_value'
          value = data[:value].rjust(12, " ")
          data[:value] = {value: value}
        when 'display_time'
          data[:value] = {time_format: data[:value]}
        when 'display_free_value'
          data[:value] = {value: data[:value]}
        when 'display_tube_animation'
          #data[:value] = {animation_name: data[:value], animation_options: data[:options]}
          data[:value] = {animation_name: data[:value]}
        when 'test'
          data[:value] = {}
      end

      data
    end

    def valid_tube_value?(s)
      s.to_s.match(/^\d{1,12}$/) == nil ? false : true
    end

  end
end