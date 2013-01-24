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
require_relative 'nixonpi/messaging/messaging'
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

      @message_distributor = NixonPi::Messaging::MessageReceiver.new

      NixonPi::MachineManager.add_state_machine(:tubes, 1, @message_distributor)
      NixonPi::MachineManager.add_state_machine(:bar, Settings.in13_pins.size, @message_distributor)
      NixonPi::MachineManager.add_state_machine(:lamp, Settings.in1_pins.size, @message_distributor)
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
      @message_distributor.add_receiver(PowerDriver.instance, :power)
      @message_distributor.add_receiver(NixonPi::Scheduler.new, :schedule)
      @message_distributor.add_receiver(NixonPi::SpeechDriver.new, :speech)
      NixonPi::MachineManager.start_state_machines
      NixonPi::MachineManager.join_threads
    end

    ##
    # quit service power down nicely
    def quit!
      log.info "Nixon Pi is shutting down..."
      NixonPi::MachineManager.exit
      NixonPi::Scheduler.exit_scheduler
      @message_distributor.on_exit
      log.info "Blow the candles out..."
      PowerDriver.instance.power_off
      log.info "Bye ;)"
      #exit(0)
      exit!
    end
  end
end


