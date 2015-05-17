module NixonPi
  class NixieService
    include Logging
    include OSInfo
    include DependencyInjection

    ActiveRecord::Base.logger = Logger.new(STDERR)

    register :in1_proxy, NixonPi::Driver::Proxy::LampProxy, ports: Settings.in1_pins
    register :in13_driver, NixonPi::Driver::PwmDriver, ports: Settings.in13_pins
    register :in12a_driver, NixonPi::Driver::TubeDriver, data: Settings.in12a_tubes.data_pin, clock: Settings.in12a_tubes.clock_pin, latch: Settings.in12a_tubes.latch_pin
    register :power, NixonPi::Driver::Proxy::PowerProxy, port: Settings.power_pin
    register :rgb_proxy, NixonPi::Driver::Proxy::RgbProxy, ports: Settings.rgb_pins
    register :background, NixonPi::Driver::Proxy::BackgroundProxy, port: Settings.background_led_pin
    register :cmd_send, NixonPi::Messaging::CommandSender

    def initialize
      log.info 'Initializing Nixon-Pi service..'
      log.info "Environment: #{ENV['RACK_ENV']}"
      ActiveRecord::Base.establish_connection(adapter: 'sqlite3', database: Settings.database)

      log.debug 'Running migrations'
      #ActiveRecord::Migrator.up('db/migrate')
      load 'db/schema.rb'


      begin
        @message_distributor = NixonPi::Messaging::CommandReceiver.new
        @info_gatherer = NixonPi::InformationProxy.new
      rescue Bunny::TCPConnectionFailed
        log.error 'RabbitMQ server not found! is it running?'
        exit!(false)
      end


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

      @message_distributor.add_receiver(NixonPi::Driver::Proxy::SoundProxy.new, :sound)
      @message_distributor.add_receiver(NixonPi::DependencyInjection::Container.get_injected(:power), :power)
      @message_distributor.add_receiver(NixonPi::Scheduler.new, :schedule)
      @message_distributor.add_receiver(NixonPi::DependencyInjection::Container.get_injected(:background), :background)

      @info_gatherer.add_target(NixonPi::HardwareInfo.new, :hardware)
      @info_gatherer.add_target(NixonPi::Driver::Proxy::SoundProxy.new, :sound)
      @info_gatherer.add_target(NixonPi::Scheduler.new, :schedule)
      @info_gatherer.add_target(NixonPi::DependencyInjection::Container.get_injected(:power), :power)
      @info_gatherer.add_target(NixonPi::DependencyInjection::Container.get_injected(:background), :background)
      @info_gatherer.add_target(@message_distributor, :commands)

      @info_gatherer.add_target(NixonPi::DependencyInjection::Container.get_class(:single_fly_in), :single_fly_in)
      @info_gatherer.add_target(NixonPi::DependencyInjection::Container.get_class(:switch_numbers), :switch_numbers)
      @info_gatherer.add_target(NixonPi::DependencyInjection::Container.get_class(:rgb_animation), :rgb_animation)
      @info_gatherer.add_target(NixonPi::DependencyInjection::Container.get_class(:ramp_up_down), :ramp_up_down)
      @info_gatherer.add_target(NixonPi::DependencyInjection::Container.get_class(:blink), :blink)

      #$SAFE = 1   # disable eval() and friends
      #DRb.start_service('druby://localhost:9001', @info_gatherer)
    end

    ##
    # Run service run
    def run!
      # use literal writing to correct speech pattern
      NixonPi::DependencyInjection::Container.get_injected(:cmd_send).send_command(:sound, value: 'Hi, my name is Nixon Pie')
      # State ip address for better connectabilty
      NixonPi::DependencyInjection::Container.get_injected(:cmd_send).send_command(:sound, value: 'My eye pee addresses are:' + NixonPi::NetworkInfo.info.join(', '))

      NixonPi::DependencyInjection::Container.get_injected(:cmd_send).send_command(:lamp0, state: :free_value, value: 0)
      NixonPi::DependencyInjection::Container.get_injected(:cmd_send).send_command(:lamp1, state: :free_value, value: 0)
      NixonPi::DependencyInjection::Container.get_injected(:cmd_send).send_command(:lamp2, state: :free_value, value: 0)
      NixonPi::DependencyInjection::Container.get_injected(:cmd_send).send_command(:lamp3, state: :free_value, value: 0)
      NixonPi::DependencyInjection::Container.get_injected(:cmd_send).send_command(:lamp4, state: :free_value, value: 0)

      NixonPi::DependencyInjection::Container.get_injected(:cmd_send).send_command(:sound, value: 'Power on!')
      NixonPi::DependencyInjection::Container.get_injected(:power).power_on

      NixonPi::MachineManager.start_state_machines
      NixonPi::MachineManager.join_threads # this must be inside the main run script - else the subthreads exit
    end

    ##
    # quit service power down nicely
    def shutdown
      log.info 'Nixon Pi is shutting down...'
      #DRb.stop_service

      #DRb.thread.join unless DRb.thread.nil?

      NixonPi::MachineManager.exit
      NixonPi::Scheduler.exit_scheduler

      log.info 'Blow the candles out...'
      NixonPi::DependencyInjection::Container.get_injected(:power).power_off
      log.info 'Bye ;)'
      @message_distributor.on_exit
    end
  end
end
