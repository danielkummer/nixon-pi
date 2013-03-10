require 'rubygems'

require_relative 'version'
require_relative 'nixonpi/delegators/multi_delegator'
require_relative 'nixonpi/configurations/state_hash'
require_relative 'nixonpi/logging/logging'
require_relative 'nixonpi/client/abio_card_client'
require_relative 'nixonpi/drivers/basic/tube_driver'
require_relative 'nixonpi/drivers/lamp_driver'
require_relative 'nixonpi/drivers/basic/pwm_driver'
require_relative 'nixonpi/drivers/background_driver'
require_relative 'nixonpi/state_machines/bar_state_machine'
require_relative 'nixonpi/state_machines/tube_state_machine'
require_relative 'nixonpi/state_machines/lamp_state_machine'
require_relative 'nixonpi/state_machines/rgb_state_machine'
require_relative 'nixonpi/animations/animation'
require_relative 'nixonpi/state_machines/machine_manager'
require_relative 'nixonpi/drivers/power_driver'
require_relative 'nixonpi/drivers/sound_driver'
require_relative 'nixonpi/scheduler'
require_relative 'nixonpi/messaging/command_receiver'
require_relative 'nixonpi/information/information_proxy'
require_relative 'nixonpi/information/os_info'
require_relative 'nixonpi/information/hardware_info'
require_relative 'nixonpi/drivers/driver_manager'
require 'thread'
require 'active_record'

Thread.abort_on_exception = true

DRBSERVER = 'druby://localhost:9001'

module NixonPi
  class NixieService
    include Logging
    include OSInfo

    ActiveRecord::Base.logger = Logger.new(STDERR)

    def initialize
      log.info "Initializing Nixon-Pi service.."
      log.info "Environment: #{$environment}"
      #system "cd #{File.dirname(__FILE__)} && rake db:migrate"
      ActiveRecord::Base.establish_connection("sqlite3:///db/settings.db")
      ActiveRecord::Migrator.up("/db/migrate")

      %w(INT TERM).each do |sig|
        Signal.trap(sig) do
          Process.kill 9, Process.pid
          #todo finish all threads
          #quit!()
        end
      end

      @message_distributor = NixonPi::Messaging::CommandReceiver.new
      @info_gatherer = NixonPi::InformationProxy.new


      NixonPi::DriverManager.register(
          {
              power: PowerDriver.new(Settings.power_pin),
              in1: NixonPi::LampDriver.new(Settings.in1_pins),
              in13: NixonPi::PwmDriver.new(Settings.in13_pins),
              in12a: NixonPi::TubeDriver.new(Settings.in12a_tubes.data_pin, Settings.in12a_tubes.clock_pin, Settings.in12a_tubes.latch_pin),
              rgb: NixonPi::PwmDriver.new(Settings.rgb_pins),
              background: NixonPi::BackgroundDriver.new(Settings.background_led_pin)
          }
      )


=begin
      @rgb_machine = NixonPi::MultiMachineProxy.new
      #big fat todo
      NixonPi::MachineManager.add_state_machines(:led, 3) do |receiver, target|
        @rgb_machine.add_state_machine(receiver)
      end
=end


      NixonPi::MachineManager.add_state_machines(:tubes) do |receiver, target|
        @message_distributor.add_receiver(receiver, target)
        @info_gatherer.add_info_holder(receiver, target)
      end
      NixonPi::MachineManager.add_state_machines(:bar, Settings.in13_pins.size) do |receiver, target|
        @message_distributor.add_receiver(receiver, target)
        @info_gatherer.add_info_holder(receiver, target)
      end
      NixonPi::MachineManager.add_state_machines(:lamp, Settings.in1_pins.size) do |receiver, target|
        @message_distributor.add_receiver(receiver, target)
        @info_gatherer.add_info_holder(receiver, target)
      end

      NixonPi::MachineManager.add_state_machines(:rgb) do |receiver, target|
        @message_distributor.add_receiver(receiver, target)
        @info_gatherer.add_info_holder(receiver, target)
      end



      @message_distributor.add_receiver(SoundDriver.new, :sound)
      @message_distributor.add_receiver(DriverManager.driver_for(:power), :power)
      @message_distributor.add_receiver(NixonPi::Scheduler.new, :schedule)
      @message_distributor.add_receiver(DriverManager.driver_for(:background), :background)

      @info_gatherer.add_info_holder(DriverManager.driver_for(:power), :power)
      @info_gatherer.add_info_holder(HardwareInfo.new, :hardware)
      @info_gatherer.add_info_holder(NixonPi::Scheduler.new, :schedule)
      @info_gatherer.add_info_holder(@message_distributor, :commands)
      @info_gatherer.add_info_holder(DriverManager.driver_for(:background), :background)

      DRb.start_service(DRBSERVER, @info_gatherer)
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
      NixonPi::Messaging::CommandSender.new.send_command(:sound, {value: "power on!"})
      DriverManager.driver_for(:power).power_on
      NixonPi::MachineManager.start_state_machines
      NixonPi::MachineManager.join_threads #this must be inside the main run script - else the subthreads exit
    end

    ##
    # quit service power down nicely
    def quit!
      log.info "Nixon Pi is shutting down..."
      DRb.stop_service
      DRb.thread.join unless DRb.thread.nil?
      NixonPi::MachineManager.exit
      NixonPi::Scheduler.exit_scheduler
      @message_distributor.on_exit
      log.info "Blow the candles out..."
      DriverManager.driver_for(:power).power_off
      log.info "Bye ;)"
      #exit(0)
      exit!
    end
  end
end


