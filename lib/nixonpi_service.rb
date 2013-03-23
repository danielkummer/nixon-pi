require 'rubygems'

require_relative 'version'
require_relative 'nixonpi/delegators/multi_delegator'
require_relative 'nixonpi/configurations/state_hash'
require_relative 'nixonpi/logging/logging'
require_relative 'nixonpi/client/abio_card_client'
require_relative 'nixonpi/drivers/basic/tube_driver'
require_relative 'nixonpi/drivers/proxies/lamp_proxy'
require_relative 'nixonpi/drivers/basic/pwm_driver'
require_relative 'nixonpi/drivers/proxies/background_proxy'
require_relative 'nixonpi/state_machines/bar_state_machine'
require_relative 'nixonpi/state_machines/tube_state_machine'
require_relative 'nixonpi/state_machines/lamp_state_machine'
require_relative 'nixonpi/state_machines/rgb_state_machine'
require_relative 'nixonpi/animations/animation'
require_relative 'nixonpi/state_machines/machine_manager'
require_relative 'nixonpi/drivers/proxies/power_proxy'
require_relative 'nixonpi/drivers/proxies/sound_proxy'
require_relative 'nixonpi/drivers/proxies/rgb_proxy'
require_relative 'nixonpi/scheduler'
require_relative 'nixonpi/messaging/command_receiver'
require_relative 'nixonpi/information/information_proxy'
require_relative 'nixonpi/information/os_info'
require_relative 'nixonpi/information/hardware_info'
require_relative 'nixonpi/drivers/hardware_driver_factory'
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
        end
      end

      NixonPi::HardwareDriverFactory.register(
          {
              in1: NixonPi::LampProxy.new(Settings.in1_pins),
              in13: NixonPi::PwmDriver.new(Settings.in13_pins),
              in12a: NixonPi::TubeDriver.new(Settings.in12a_tubes.data_pin, Settings.in12a_tubes.clock_pin, Settings.in12a_tubes.latch_pin),
              power: PowerProxy.new(Settings.power_pin),
              rgb: NixonPi::RgbProxy.new(Settings.rgb_pins),
              background: NixonPi::BackgroundProxy.new(Settings.background_led_pin)
          }
      )

      @message_distributor = NixonPi::Messaging::CommandReceiver.new
      @info_gatherer = NixonPi::InformationProxy.new

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


      #todo can this somehow be injected?
      @message_distributor.add_receiver(SoundProxy.new, :sound)
      @message_distributor.add_receiver(HardwareDriverFactory.instance_for(:power), :power)
      @message_distributor.add_receiver(NixonPi::Scheduler.new, :schedule)
      @message_distributor.add_receiver(HardwareDriverFactory.instance_for(:background), :background)

      @info_gatherer.add_info_holder(SoundProxy.new, :sound)
      @info_gatherer.add_info_holder(HardwareDriverFactory.instance_for(:power), :power)
      @info_gatherer.add_info_holder(HardwareInfo.new, :hardware)
      @info_gatherer.add_info_holder(NixonPi::Scheduler.new, :schedule)
      @info_gatherer.add_info_holder(HardwareDriverFactory.instance_for(:background), :background)
      @info_gatherer.add_info_holder(@message_distributor, :commands)

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
      HardwareDriverFactory.instance_for(:power).power_on
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
      HardwareDriverFactory.instance_for(:power).power_off
      log.info "Bye ;)"
      #exit(0)
      exit!
    end
  end
end


