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

require 'nixon_pi/rabbit_mq/command_sender'
require 'nixon_pi/rabbit_mq/information_sender'

module NixonPi
  class WebServer < Sinatra::Base
    set :root, File.dirname(__FILE__)

    register Sinatra::ActiveRecordExtension

    helpers Sinatra::FormHelpers
    helpers Sinatra::Jsonp

    use Rack::MethodOverride

    set :database, adapter: 'sqlite3', database: Settings.database
    set :public_folder, File.join(File.dirname(__FILE__), 'public')
    set :haml, format: :html5

    #Only process one request at a time
    #We need this, else rabbitmq doesn't work correctly, more specific the rpc call with InformationSender
    set :lock, true

    disable :raise_errors
    disable :show_exceptions

    # set :static, $environment == 'development' ? false : true
    # set :show_exceptions, false

    not_found do
      if request.accept? 'text/html'
        haml 'errors/not_found'.to_sym, layout: false, locals: {
                                          info: {
                                              title: "404 - are you sure it's there?",
                                              message: 'Nothing found!'
                                          }
                                      }
      else
        content_type :json
        status 404
        halt({success: 'false', message: "404 - are you sure it's there?"}.to_json)
      end
    end

    error Bunny::TCPConnectionFailed do
      if request.accept? 'text/html'
        haml 'errors/rabbitmq_down'.to_sym, layout: false, locals: {
                                              info: {
                                                  title: 'Oops - I found a dead bunny...',
                                                  message: 'Make sure your RabbitMQ server is up and running...'
                                              }
                                          }
      else
        content_type :json
        halt({success: 'false', message: 'Make sure your RabbitMQ server is up and running...'}.to_json)
      end
    end

    error do
      if request.accept? 'application/json'
        content_type :json

        halt({success: 'false', message: $ERROR_INFO.message}.to_json)
      end
      haml 'errors/rabbitmq_down'.to_sym, layout: false, locals: {
                                            info: {
                                                title: 'Something just went terribly wrong...',
                                                message: "Here's a funny error:#{$ERROR_INFO.message}"
                                            }
                                        }
    end

    helpers do
      def sender
        @sender ||= NixonPi::RabbitMQ::CommandSender.new
      end

      def information_sender
        @information_sender ||= NixonPi::RabbitMQ::InformationSender.instance
      end

      INDENT = '  ' # use 2 spaces for indentation
      def hash_to_haml(hash, level = 0)
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
        # TODO: add .well div do list when rendering
        result = %w(%dl)
        [:state, :value, :animation_name, :options].each do |key|
          value = data.send(key)
          unless value.nil? and value != ''
            result << "  %dt #{key.to_s.gsub('_', ' ').capitalize}"
            result << "  %dd #{value}"
          end
        end unless data.nil?
        Haml::Engine.new(result.join("\n")).render
      end

      def version
        NixonPi::VERSION.to_s
      end
    end

    ###
    # GET REQUESTS
    ###
    get '/' do
      sender.client # trigger 500 error if rabbitmq is down

      @no_of_bars = Settings.in13_pins.size
      @no_of_lamps = Settings.in1_pins.size
      %w(tubes bar0 bar1 bar2 bar3 lamp1 lamp2 lamp3 lamp4 lamp5 background rgb ).each do |target|
        initial = Command.where('target LIKE ?', target).first || Command.new(target: target)
        instance_variable_set("@#{target}", initial)
      end
      haml :control, format: :html5
    end

    get '/scheduler.?:format?' do
      @schedules = Schedule.find(:all)
      haml :scheduler, format: :html5
    end

    get '/command/:target.:format?' do
      target = params[:target]
      data = get_remote_info_from(target, :commands)
      formatted_response('json', data, "Options for command #{target}")
    end

    get '/receivers.:format?' do
      # get all available targets for commands
      data = get_remote_info_from(:commands, :receivers)
      formatted_response('json', data, 'available command receivers')
    end

    get '/information/:target/?.:format?' do
      target = params[:target]
      what = target == 'hardware' ? :io_card : :params
      data = get_remote_info_from(target, what)
      formatted_response(params[:format], data, "#{target} information")
    end

    get '/information/:target/:id.:format?' do
      target = "#{params[:target]}#{params[:id]}"
      data = get_remote_info_from(target, :params)
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

    get '/state.:format' do
      data = {}
      data[:rabbitmq] = sender.connected?
      data[:service] = get_remote_info_from(:power, :params)[:value]
      formatted_response(params[:format], data, 'application state')
    end

    ###
    # POST REQUESTS
    ###

    post '/tubes/?' do
      @params[:target] = :tubes
      preprocess_post_params(:tubes, @params) do |data|
        sender.send_command(:tubes, data)
        formatted_response('json', data, 'Tubes set to')
      end
    end

    post '/lamp/?' do
      id = params[:id]
      @params['value'] = '0' unless @params['value']
      @params[:target] = "lamp#{id}".to_sym
      preprocess_post_params(:lamp, @params) do |data|
        sender.send_command("lamp#{id}".to_sym, data)
        formatted_response('json', data, 'Lamps set to')
      end
    end

    post '/bar/?' do
      # TODO: error when no id
      id = params[:id]
      @params[:target] = "bar#{id}".to_sym
      preprocess_post_params(:bars, @params) do |data|
        sender.send_command("bar#{id}".to_sym, data)
        formatted_response('json', data, "Bar #{id} set to")
      end
    end

    post '/rgb' do
      # TODO: error when no id
      @params[:target] = :rgb
      preprocess_post_params(:rgb, @params) do |data|
        sender.send_command('rgb'.to_sym, data)
        formatted_response('json', data, 'RGB set to')
      end
    end

    post '/background' do
      @params[:target] = :background
      preprocess_post_params(:background, @params) do |data|
        sender.send_command('background'.to_sym, data)
        formatted_response('json', data, 'Background set to')
      end
    end

    post '/scheduler/?' do
      preprocess_post_params(:scheduler, @params) do |data|
        # convert json to hash
        command = data[:command]
        command = JSON.parse(command.gsub(/^\[/, '{').gsub!(/\]$/, '}')) unless command.is_a?(Hash)
        data[:command] = command
        sender.send_command(:schedule, data)
        formatted_response('json', data, 'Command scheduled')
      end
    end

    post '/say/?' do
      @params[:target] = :sound
      preprocess_post_params(:sound, @params) do |data|
        sender.send_command(:sound, data)
        formatted_response('json', data, 'Speak ')
      end
    end

    post '/power/?', provides: [:json] do
      @params[:value] = 0 if @params.empty?
      @params[:target] = :power
      preprocess_post_params(:power, @params) do |data|
        sender.send_command(:power, data)
        formatted_response('json', data, 'Power set to')
      end
    end

    ###
    # DELETE REQUESTS
    ###

    delete '/schedule/:id/?' do |id|
      schedule = Schedule.find_by_id(id)
      if schedule
        data = {}
        data[:delete] = true
        data[:id] = schedule.id
        sender.send_command(:schedule, data)
        schedule.destroy
        set_message!(data, 'Schedule removed', false)
      else
        set_message!(data, 'Schedule not fount', false)
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

      # find possible json strings values and convert to hash
      data.each do |k, v|
        data[k] = JSON.parse(v) if v.to_s.starts_with?('{')
      end
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
          data.delete(:state_machine)
          Schedule.new(data) # schedules are always newly created - you can only delete existing ones
        else
          data.delete(:state_machine)
          get_or_create_command(data)
      end
    end

    ##
    # Get a command object or create one, also do some minor value adjustments
    # @param [Hash] data
    def get_or_create_command(data)
      case data[:target].to_sym
        when :tubes
          data[:value] = data[:value].to_s.rjust(12, ' ') unless data[:value].nil?
          data[:value] = data[:time_format] if data[:state].to_sym == :time
          data.delete(:time_format)
        else
          data.delete(:id) # not intended for ar - use

      end
      command = nil
      if params[:initial]
        initial = Command.find(:first, conditions: ['target = ?', data[:target].to_s])
        command = initial if !initial.nil? && initial.update_attributes(data)
      end
      command ||= Command.new(data)
      command
    end

    ##
    # Return a formated response object with an optional response message
    # @param [Symbol] format json or html
    # @param [Hash] data
    # @param [String] respond_message
    def formatted_response(format, data, respond_message = '')

      if data.is_a? Array
        data = {data: data}
      end

      data ||= {}
      #data can be an array!!

      data[:message] ||= []
      data[:success] = true unless data.key?(:success)
      data[:message] << respond_message unless respond_message.empty?

      case format.to_sym
        when :json
          data.delete(:time)
          halt jsonp(data)
        when :html
          halt haml(:formatted_response, locals: data)
        else
          error 406
          set_message!(data, 'Unknown format', false)
          halt jsonp(data)
      end
    end

    ##
    # RPC Connection to service, get data from InformationInbox
    # @param [Symbol] target information target
    # @param [Symbol] about regested information identifier
    def get_remote_info_from(_target, _about)
      information_sender.get_info_from(_target, {about: _about})
    end

  end
end

# TODO: do we need DRb.thread.join ?
