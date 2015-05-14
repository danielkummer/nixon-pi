#http://www.michaelrigart.be/en/blog/a-simple-ruby-command-line-tool.html

#require 'nixon_pi'
#http://recipes.sinatrarb.com/p/embed/event-machine

require 'thin'

module NixonPi
  module Cli
    class Application < Thor
      package_name 'NixonPi'
      #map '-L' => :list

      option :environment, default: 'development', banner: '<environment>'
      option :mock, type: :boolean, banner: 'Use telnet mock implementation', default: false
      option :p, type: :numeric, banner: 'Set webserver port', default: 8080
      desc 'start', 'starts the background service'
      def start

        ENV['NIXON_PI_FORCE_MOCK'] = 'true' if options[:mock]

        #NixonPi::WebServer.run!



           opts = {}



        EventMachine.run do

          server  = opts[:server] || 'thin'
          host    = opts[:host]   || '0.0.0.0'
          port    = opts[:port]   || '8181'
          #web_app = opts[:app]
          web_app = NixonPi::WebServer.new

          dispatch = Rack::Builder.app do
            map '/' do
              run web_app
            end
          end

          unless ['thin', 'hatetepe', 'goliath'].include? server
            raise "Need an EM webserver, but #{server} isn't"
          end

          # Start the web server. Note that you are free to run other tasks
          # within your EM instance.
          Rack::Server.start({
                                 app:    dispatch,
                                 server: server,
                                 Host:   host,
                                 Port:   port,
                                 signals: false,
                             })


          EventMachine.defer do
            server = NixonPi::NixieService.new
            server.run!
          end
          #EM::WebSocket.start(:host => '0.0.0.0', :port => 3001) do
            # Websocket code here
          #end



          #NixonPi::WebServer.run!({port: 3000})
        end






      end

      desc 'Install', 'Install application'

      def install

      end


    end
  end
end
