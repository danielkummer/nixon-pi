module NixonPi
  module Driver
    module Proxy
      class PowerProxy
        include Logging
        include Commands
        include InformationHolder

        accepted_commands :value

        def initialize(options = { port: nil })
          @io_driver = IoDriver.new([options[:port]])
          @value = 0
          log.info 'Initializing power driver...'
        end

        def handle_command(command)
          value = command[:value].to_i
          log.debug "got power command: #{command}, applying..."
          if (0..1).member?(value)
            NixonPi::DependencyInjection::Container.get_injected(:cmd_send).send_command(:sound, value: "power #{value == 1 ? 'on' : 'off'}!")
            @io_driver.write(value)
            @value = value
          end
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

        def power_on
          @value = 1
          @io_driver.write(1)
        end

        def power_off
          @value = 0
          @io_driver.write(0)
        end

        def on?
          @value == 1 ? true : false
        end

        def get_params
          { value: @value }
        end
      end
    end
  end
end
