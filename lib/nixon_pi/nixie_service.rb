module NixonPi
  class NixieService
    include Logging
    include OSInfo

    ActiveRecord::Base.logger = Logger.new(STDERR)

    register :in1_proxy, NixonPi::Driver::Proxy::LampProxy, ports: Settings.in1_pins
    register :in13_driver, NixonPi::Driver::PwmDriver, ports: Settings.in13_pins
    register :in12a_driver, NixonPi::Driver::TubeDriver, data: Settings.in12a_tubes.data_pin, clock: Settings.in12a_tubes.clock_pin, latch: Settings.in12a_tubes.latch_pin
    register :power, NixonPi::Driver::Proxy::PowerProxy, port: Settings.power_pin
    register :rgb_proxy, NixonPi::Driver::Proxy::RgbProxy, ports: Settings.rgb_pins
    register :background, NixonPi::Driver::Proxy::BackgroundProxy, port: Settings.background_led_pin

    def initialize
      log.info 'Initializing Nixon-Pi service..'
      log.info "Environment: #{ENV['RACK_ENV']}"

      ActiveRecord::Base.establish_connection(adapter: 'sqlite3', database: Settings.database)
      ActiveRecord::Migrator.up('/db/migrate')

      # Kill process if interrupted while initializing
      %w(INT TERM).each { |sig| Signal.trap(sig) { Process.kill 9, Process.pid } }

      @message_distributor = NixonPi::Messaging::CommandReceiver.new
      @info_gatherer = NixonPi::InformationProxy.new

      NixonPi::MachineManager.add_state_machines(:tubes) do |receiver, target|
        @message_distributor.add_receiver(receiver, target)
        @info_gatherer.add_target(receiver, target)
      end
      NixonPi::MachineManager.add_state_machines(:bar, Settings.in13_pins.size) do |receiver, target|
        @message_distributor.add_receiver(receiver, target)
        @info_gatherer.add_target(receiver, target)
      end
      NixonPi::MachineManager.add_state_machines(:lamp, Settings.in1_pins.size) do |receiver, target|
        @message_distributor.add_receiver(receiver, target)
        @info_gatherer.add_target(receiver, target)
      end

      NixonPi::MachineManager.add_state_machines(:rgb) do |receiver, target|
        @message_distributor.add_receiver(receiver, target)
        @info_gatherer.add_target(receiver, target)
      end

      @message_distributor.add_receiver(SoundProxy.new, :sound)
      @message_distributor.add_receiver(get_injected(:power), :power)
      @message_distributor.add_receiver(NixonPi::Scheduler.new, :schedule)
      @message_distributor.add_receiver(get_injected(:background), :background)

      @info_gatherer.add_target(HardwareInfo.new, :hardware)
      @info_gatherer.add_target(SoundProxy.new, :sound)
      @info_gatherer.add_target(NixonPi::Scheduler.new, :schedule)
      @info_gatherer.add_target(get_injected(:power), :power)
      @info_gatherer.add_target(get_injected(:background), :background)
      @info_gatherer.add_target(@message_distributor, :commands)

      @info_gatherer.add_target(get_class(:single_fly_in), :single_fly_in)
      @info_gatherer.add_target(get_class(:switch_numbers), :switch_numbers)
      @info_gatherer.add_target(get_class(:rgb_animation), :rgb_animation)
      @info_gatherer.add_target(get_class(:ramp_up_down), :ramp_up_down)
      @info_gatherer.add_target(get_class(:blink), :blink)

      DRb.start_service('druby://localhost:9001', @info_gatherer)
    end

    ##
    # Run service run
    def run!
      [:INT, :TERM].each do |sig|
        trap(sig) do
          shutdown
        end
      end
      # use literal writing to correct speech pattern
      NixonPi::Messaging::CommandSender.new.send_command(:sound, value: 'Hi, my name is Nixon Pie')
      # State ip address for better connectabilty
      NixonPi::Messaging::CommandSender.new.send_command(:sound, value: 'My eye pee addresses are:' + NixonPi::NetworkInfo.info.join(', '))

      NixonPi::Messaging::CommandSender.new.send_command(:lamp0, state: :free_value, value: 0)
      NixonPi::Messaging::CommandSender.new.send_command(:lamp1, state: :free_value, value: 0)
      NixonPi::Messaging::CommandSender.new.send_command(:lamp2, state: :free_value, value: 0)
      NixonPi::Messaging::CommandSender.new.send_command(:lamp3, state: :free_value, value: 0)
      NixonPi::Messaging::CommandSender.new.send_command(:lamp4, state: :free_value, value: 0)

      NixonPi::Messaging::CommandSender.new.send_command(:sound, value: 'Power on!')
      get_injected(:power).power_on

      NixonPi::MachineManager.start_state_machines
      NixonPi::MachineManager.join_threads # this must be inside the main run script - else the subthreads exit
    end

    ##
    # quit service power down nicely
    def shutdown
      log.info 'Nixon Pi is shutting down...'
      DRb.stop_service

      DRb.thread.join unless DRb.thread.nil?

      NixonPi::MachineManager.exit
      NixonPi::Scheduler.exit_scheduler

      @message_distributor.on_exit
      log.info 'Blow the candles out...'
      get_injected(:power).power_off
      log.info 'Bye ;)'
      exit!(true)
    end
  end
end
