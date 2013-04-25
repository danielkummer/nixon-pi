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
require_relative 'dependency'

require_relative 'nixonpi/animations/tube/count_up_all_animation'
require_relative 'nixonpi/animations/tube/switch_numbers_animation'
require_relative 'nixonpi/animations/lamp/blink_animation'
require_relative 'nixonpi/animations/led/rgb_animation'
require_relative 'nixonpi/animations/tube/single_fly_in_animation'
require_relative 'nixonpi/animations/bar/ramp_up_down_animation'

require 'thread'
require 'active_record'

Thread.abort_on_exception = true

DRBSERVER = 'druby://localhost:9001'

module NixonPi
  class NixieService
    include Logging
    include OSInfo

    ActiveRecord::Base.logger = Logger.new(STDERR)

    register :in1_proxy, NixonPi::LampProxy, ports: Settings.in1_pins
    register :in13_driver, NixonPi::PwmDriver, ports: Settings.in13_pins
    register :in12a_driver, NixonPi::TubeDriver, data: Settings.in12a_tubes.data_pin, clock: Settings.in12a_tubes.clock_pin, latch: Settings.in12a_tubes.latch_pin
    register :power, NixonPi::PowerProxy, port: Settings.power_pin
    register :rgb_proxy, NixonPi::RgbProxy, ports: Settings.rgb_pins
    register :background, NixonPi::BackgroundProxy, port: Settings.background_led_pin

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
      @message_distributor.add_receiver(get_injected(:power), :power)
      @message_distributor.add_receiver(NixonPi::Scheduler.new, :schedule)
      @message_distributor.add_receiver(get_injected(:background), :background)

      @info_gatherer.add_info_holder(SoundProxy.new, :sound)
      @info_gatherer.add_info_holder(get_injected(:power), :power)
      @info_gatherer.add_info_holder(HardwareInfo.new, :hardware)
      @info_gatherer.add_info_holder(NixonPi::Scheduler.new, :schedule)
      @info_gatherer.add_info_holder(get_injected(:background), :background)
      @info_gatherer.add_info_holder(@message_distributor, :commands)

      #todo how do i get this to work, they need options to function...

      @info_gatherer.add_info_holder(get_class(:single_fly_in), :single_fly_in)
      @info_gatherer.add_info_holder(get_class(:switch_numbers), :switch_numbers)
      @info_gatherer.add_info_holder(get_class(:rgb_animation), :rgb_animation)
      @info_gatherer.add_info_holder(get_class(:ramp_up_down), :ramp_up_down)
      @info_gatherer.add_info_holder(get_class(:blink), :blink)

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
      NixonPi::Messaging::CommandSender.new.send_command(:sound, {value: "Hi Im Nixon pi"})
      #get_injected(:power).power_on
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
      get_injected(:power).power_off
      log.info "Bye ;)"
      #exit(0)
      exit!
    end
  end
end


