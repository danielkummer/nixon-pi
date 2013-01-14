require 'singleton'
require_relative 'driver'
require_relative '../configurations/settings'

module NixonPi
  class PowerDriver
    include Logging
    include Singleton
    include Driver

    def initialize
      @power_pin = Settings.power_pin
      @value = 0
      log.info "Initializing power pin: #{@power_pin.to_s}"
      CommandProcessor.add_receiver(self, :power)
    end

    def receive(command)
      value = command[:value]
      log.debug "got power command: #{command}, applying..."
      if (0..1).member?(value)
        client.io_write(@power_pin, value)
      end
    end

    def power_on
      client.io_write(@power_pin, 1)
    end

    def power_off
      client.io_write(@power_pin, 0)
    end

    # Currently unused..
    def write(power_on = false)
      write = power_on ? 1 : 0
      client.io_write(@power_pin, write)
    end

    def on?
      @value == 1 ? true : false
    end

  end
end