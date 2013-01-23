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
require_relative 'nixonpi/state_machines/machine_manager'
require_relative 'nixonpi/drivers/power_driver'
require_relative 'nixonpi/drivers/speech_driver'
require_relative 'nixonpi/scheduler'
require_relative 'os'
require 'thread'
#require 'daemons'

require 'ruby-prof' if $environment == "development"


Thread.abort_on_exception = true

module NixonPi
  class NixieService
    include Logging
    include OS


    ActiveRecord::Base.logger = Logger.new(STDERR)

    def initialize
      RubyProf.start if $environment == "development"
      log.info "Initializing Nixon-Pi service.."
      log.info "Environment: #{$environment}"
      system "cd #{File.dirname(__FILE__)} && rake db:migrate"

      %w(INT TERM).each do |sig|
        Signal.trap(sig) do
          Process.kill 9, Process.pid
          #todo finish all threads
          #quit!()
        end
      end

      NixonPi::MachineManager.add_state_machine(:tubes)
      NixonPi::MachineManager.add_state_machine(:bar, Settings.in13_pins.size)
      NixonPi::MachineManager.add_state_machine(:lamp, Settings.in1_pins.size)
    end

    ##
    # Run service run
    def run!
      # Become a daemon
      #Daemons.daemonize if $environment == 'production'

      [:INT, :TERM].each do |sig|
        trap(sig) do
          #todo finish all threads
          quit!()
        end
      end

      log.info "Start running..."
      log.info "turn on power"
      PowerDriver.instance.power_on
      CommandQueue.add_receiver(NixonPi::Scheduler.instance, :schedule)
      CommandQueue.add_receiver(NixonPi::SpeechDriver.instance, :speech)
      NixonPi::MachineManager.start_state_machines
      NixonPi::MachineManager.join_threads
    end

    ##
    # quit service power down nicely
    def quit!
      log.info "Nixon Pi is shutting down..."
      NixonPi::MachineManager.exit
      NixonPi::Scheduler.exit_scheduler
      log.info "Turning off power"
      PowerDriver.instance.power_off
      CommandQueue.at_exit
      log.info "Bye ;)"
      #exit(0)
      exit!
    end
  end
end


