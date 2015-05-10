require_relative '../../messaging/commands_module'
require_relative '../../messaging/command_receiver'
require_relative '../../information/information_holder'
require_relative '../basic/pwm_driver'
require_relative '../driver'

module NixonPi
  class BackgroundProxy
    include Logging
    include Commands
    include InformationHolder

    accepted_commands :value

    def initialize(options = { port: nil })
      @pwm_driver = PwmDriver.new(ports: [options[:port]])
      log.info 'Initializing background driver...'
      @value = 0
    end

    def handle_command(command)
      value = command[:value].to_i
      log.debug "got background command: #{command}, applying..."
      @pwm_driver.write_to_port(0, value)
      @value = value
    end

    def handle_info_request(about)
      ret = {}
      case about.to_sym
        when :params
          ret = { value: @value }
        when :commands
          ret = { commands: self.class.available_commands }
        else
          log.error "No information about #{about}"
      end
      ret
    end

    def get_params
      { value: @value }
    end
  end
end
