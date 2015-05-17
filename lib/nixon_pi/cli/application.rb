#http://www.michaelrigart.be/en/blog/a-simple-ruby-command-line-tool.html

#require 'nixon_pi'
#http://recipes.sinatrarb.com/p/embed/event-machine

require 'thin'

module NixonPi
  module Cli
    class Application < Thor
      package_name 'NixonPi'
      #map '-L' => :list

      long_desc <<-LONGDESC
            Starts the nixon-pi service and webservice

            With -m option, the connection to the abiocard will be mocked

            With -p option, you can supply a port for the webservice

            With --environment option, the environment can be specified - development, production
      LONGDESC
      option :environment, default: 'development', banner: '<environment>'
      option :m, type: :boolean, banner: 'Use telnet mock implementation', default: false, tags: :boolean
      option :p, type: :numeric, banner: 'Set webserver port', default: 8080
      desc 'start', 'starts the service'

      def start
        ENV['RACK_ENV'] = options[:environment] if options[:environment]
        ENV['NIXON_PI_FORCE_MOCK'] = 'true' if options[:m]
        NixonPi::Runner.new.run(options)
      end

      long_desc <<-LONGDESC
            Starts the web service

            With -p option, you can supply a port for the webservice

            With --environment option, the environment can be specified - development, production
      LONGDESC
      desc 'web', 'start web server instance using thin'
      option :environment, default: 'development', banner: '<environment>'
      option :p, type: :numeric, banner: 'Set webserver port', default: 8080

      def web
        NixonPi::WebServer.set :environment, options[:environment] if options[:environment]
        NixonPi::WebServer.set :port, options[:p].to_s if options[:p]
        NixonPi::WebServer.run!
      end


      desc 'install', 'install runfiles'
      #todo dynamic paths!
      def install
        runfile = system('rvmsudo bundle exec foreman export bluepill ./config -f ./Procfile.production -a nixon-pi -u root -l /home/pi/nixonpi/shared/log')
        log.error 'Unable to install runfile!' unless runfile
      end
    end
  end
end
