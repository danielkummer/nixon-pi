require 'singleton'
require_relative 'driver'
require_relative '../configurations/settings'
require_relative '../messaging/command_listener'
require_relative '../messaging/command_receiver'
require_relative '../information/information_holder'


module NixonPi
  class PowerDriver
    include Logging
    include Singleton
    include Driver
    include CommandListener
    include InformationHolder

    accepted_commands :value

    def initialize
      @power_driver = IODriver.new(Settings.power_pin)

      @value = 0
      log.info "Initializing power pin"
    end

    def handle_command(command)
      value = command[:value].to_i
      log.debug "got power command: #{command}, applying..."
      if (0..1).member?(value)
        NixonPi::Messaging::CommandSender.new.send_command(:sound, {value: "power #{value == 1 ? "on" : "off"}!"})
        client.io_write(@power_pin, value)
        @value = value
      end
    end

    def handle_info_request(about)
      ret = Hash.new
      case about.to_sym
        when :params
          ret = {value: @value}
        when :commands
          ret = {commands: self.class.available_commands}
        else
          log.error "No information about #{about}"
      end
      ret
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