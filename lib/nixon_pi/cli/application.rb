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
        NixonPi::Runner.run(options)
      end
    end
  end
end
