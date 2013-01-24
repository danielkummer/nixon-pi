require 'singleton'
require_relative 'driver'
require_relative '../configurations/settings'
require_relative '../messaging/message_listener'
require_relative '../messaging/messaging'


module NixonPi
  class PowerDriver
    include Logging
    include Singleton
    include Driver
    include MessageListener

    accepted_commands :value

    def initialize
      @power_pin = Settings.power_pin
      @value = 0
      log.info "Initializing power pin: #{@power_pin.to_s}"
    end

    def handle_command(command)
      value = command[:value]
      log.debug "got power command: #{command}, applying..."
      if (0..1).member?(value)
        client.io_write(@power_pin, value)
        @value = value
      end
    end

    def handle_inquiry(about)
      return @value
    end

    def power_on
      @value = 1
      client.io_write(@power_pin, 1)
    end

    def power_off
      @value = 0
      client.io_write(@power_pin, 0)
    end

    # Currently unused..
    def write(power_on = false)
      write = power_on ? 1 : 0
      client.io_write(@power_pin, write)
      @value = write
    end

    def on?
      @value == 1 ? true : false
    end

    def get_params
      {value: @value}
    end

  end
end