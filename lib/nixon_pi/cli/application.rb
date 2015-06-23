require 'thin'

module NixonPi
  module Cli
    class Application < Thor
      package_name 'NixonPi'
      # map '-L' => :list

      long_desc <<-LONGDESC
            Starts the nixon-pi service

            With -m option, the connection to the abiocard will be mocked

            With -p option, you can supply a port for the webservice

            With --environment option, the environment can be specified - development, production

            With --db option,  set the path for the database, the file nixon_pi.sqlite3 will be stored there
      LONGDESC
      option :env, default: 'development', banner: '<environment>'
      option :m, type: :boolean, banner: 'Use telnet mock implementation', default: false, tags: :boolean
      option :p, type: :numeric, banner: 'Set webserver port', default: 8080
      option :u, type: :numeric, banner: 'Set upd server port', default: 1234
      option :db, type: :string, banner: '<path>'
      desc 'service', 'starts the service'
      def service
        $environment = options[:env]
        $force_mock = true if options[:m]
        $upd_port = options[:u]

        NixonPi::Settings.reload! # make sure we have the correct settings for the correct environment!
        NixonPi::Settings['database_path'] = options[:db] if options[:db]
        NixonPi::Runner.new.run(options)
      end

      long_desc <<-LONGDESC
            Starts the web service

            With -p option, you can supply a port for the webservice

            With --environment option, the environment can be specified - development, production

            With --db option,  set the path for the database, the file nixon_pi.sqlite3 will be stored there
      LONGDESC
      desc 'web', 'start web server instance using thin'
      option :env, default: 'development', banner: '<environment>'
      option :p, type: :numeric, banner: 'Set webserver port', default: 8080
      option :db, type: :string, banner: '<path>'
      def web
        $environment = options[:env]

        NixonPi::Settings.reload!
        NixonPi::WebServer.set :environment, options[:env]
        NixonPi::WebServer.set :port, options[:p].to_s
        NixonPi::WebServer.run!
      end

      desc 'install', 'install runfiles'
      # TODO: dynamic paths! and complete installation
      def install
        runfile = system('rvmsudo bundle exec foreman export bluepill ./config -f ./Procfile.production -a nixon-pi -u root -l /home/pi/nixonpi/shared/log')
        log.error 'Unable to install runfile!' unless runfile
      end
    end
  end
end
