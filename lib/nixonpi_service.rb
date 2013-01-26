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
require_relative 'nixonpi/information/information_proxy'
require_relative 'nixonpi/information/os_info'
require_relative 'nixonpi/information/hardware_info'
require 'thread'
#require 'daemons'

require 'ruby-prof' if $environment == "development"


Thread.abort_on_exception = true

DRBSERVER = 'druby://localhost:9001'

module NixonPi
  class NixieService
    include Logging
    include OSInfo


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

      @message_distributor = NixonPi::Messaging::CommandReceiver.new
      @info_gatherer = NixonPi::InformationProxy.new

=begin
DRb.start_service 'druby://:9000', Counter.new
 puts "Server running at #{DRb.uri}"

 trap("INT") { DRb.stop_service }
 DRb.thread.join

use drb to read params from state machines
=end

      NixonPi::MachineManager.add_state_machine(:tubes, 1) do |receiver, target|
        @message_distributor.add_receiver(receiver, target)
        @info_gatherer.add_info_holder(receiver, target)
      end
      NixonPi::MachineManager.add_state_machine(:bar, Settings.in13_pins.size) do |receiver, target|
        @message_distributor.add_receiver(receiver, target)
        @info_gatherer.add_info_holder(receiver, target)
      end
      NixonPi::MachineManager.add_state_machine(:lamp, Settings.in1_pins.size) do |receiver, target|
        @message_distributor.add_receiver(receiver, target)
        @info_gatherer.add_info_holder(receiver, target)
      end

      @message_distributor.add_receiver(SpeechDriver.new, :speech)
      @message_distributor.add_receiver(PowerDriver.instance, :power)
      @info_gatherer.add_info_holder(PowerDriver.instance, :power)
      @info_gatherer.add_info_holder(HardwareInfo.new, :hardware)
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
      PowerDriver.instance.power_on

      @message_distributor.add_receiver(NixonPi::Scheduler.new, :schedule)
      @info_gatherer.add_info_holder(NixonPi::Scheduler.new, :schedule)
      DRb.start_service( DRBSERVER, @info_gatherer )

      NixonPi::MachineManager.start_state_machines

      NixonPi::MachineManager.join_threads #this must be inside the main run script - else the subthreads exit
    end

    ##
    # quit service power down nicely
    def quit!
      log.info "Nixon Pi is shutting down..."

      DRb.stop_service
      DRb.thread.join
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


