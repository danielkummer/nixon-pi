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
require_relative 'nixonpi/drivers/power_driver'
require_relative 'nixonpi/command_processor'
require_relative 'nixonpi/scheduler'
require 'thread'
require 'daemons'


Thread.abort_on_exception = true

module NixonPi
  class NixieService
    include Logging

    ActiveRecord::Base.logger = Logger.new(STDERR)

    def initialize
      log.info "Initializing Nixon-Pi service.."
      log.info "Environment: #{$environment}"
      system "cd #{File.dirname(__FILE__)} && rake db:migrate"

      NixonPi::MachineManager.add_state_machines(:tubes, :bars, :lamps)
      @server = WebServer
    end

    ##
    # Run service run
    def run!
      # Become a daemon
      Daemons.daemonize if $environment == 'production'

      [:INT, :TERM].each do |sig|
        trap(sig) do
          #todo finish all threads
          quit!()
        end
      end

      log.info "Start running..."
      log.info "turn on power"
      PowerDriver.instance.power_on
      @web_thread = Thread.new { @server.run! }
      CommandProcessor.add_receiver(NixonPi::Scheduler.instance, :schedule)
      NixonPi::MachineManager.start_state_machines
      NixonPi::CommandProcessor.start
      NixonPi::MachineManager.join_threads
      NixonPi::CommandProcessor.join_thread
    end

    ##
    # quit service power down nicely
    def quit!
      log.info "Nixon Pi is shutting down..."
      Thread.kill(@web_thread)
      NixonPi::MachineManager.exit
      NixonPi::CommandProcessor.exit
      NixonPi::Scheduler.exit
      log.info "Turning off power"
      PowerDriver.instance.power_off
      log.info "Bye ;)"
      exit(0)
    end
  end
end


