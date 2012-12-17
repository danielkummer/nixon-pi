require 'rubygems'

require_relative 'nixonpi/version'
require_relative 'nixonpi/delegators/multi_delegator'
require_relative 'nixonpi/configurations/state_hash'
require_relative 'nixonpi/logging/logging'
require_relative 'nixonpi/client/abio_card_client'
require_relative 'nixonpi/drivers/tube_driver'
require_relative 'nixonpi/drivers/lamp_driver'
require_relative 'nixonpi/drivers/bar_graph_driver'
require_relative 'nixonpi/state_machines/bar_state_machine'
require_relative 'nixonpi/state_machines/tube_state_machine'
require_relative 'nixonpi/state_machines/lamp_state_machine'
require_relative 'nixonpi/animations/animation'
require_relative 'nixonpi/web/web_server'
require_relative 'nixonpi/state_machines/machine_manager'


module NixonPi
  class NixieService
    include Logging

    ActiveRecord::Base.logger = Logger.new(STDERR)


    def initialize
      log.info "Initializing Nixon-Pi service.."
      log.info "Environment: #{$environment}"
      system "cd #{File.dirname(__FILE__)} && rake db:migrate"

      ##load saved values if any...

      NixonPi::MachineManager.add_state_machines(:tubes, :bars, :lamps)
      @server = WebServer
    end

    ##
    # Run service run
    def run!
      [:INT, :TERM].each do |sig|
        trap(sig) { quit!() }
      end

      log.info "Start running..."
      @web_thread = Thread.new { @server.run! }
      NixonPi::MachineManager.start_state_machines
      NixonPi::MachineManager.join_threads
    end

    def quit!
      log.info "Nixon Pi is shutting down..."
      Thread.kill(@web_thread)

      log.info "Bye ;)"
      exit(0)
    end
  end
end


