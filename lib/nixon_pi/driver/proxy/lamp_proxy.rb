module NixonPi
  module Driver
    module Proxy
      class LampProxy
        include Logging

        def initialize(options = {ports: nil})
          log.info 'Initializing lamps driver'
          @pwm_driver = PwmDriver.new(options)
        end

        ##
        # Write to a single lamp
        # @param [Integer] number
        # @param [Integer] value 0 or > 1
        def write_to_lamp(number, value)
          value = value.to_i >= 1 ? 255 : 0
          @pwm_driver.write_to_port(number, value)
        end

        ##
        # Write multiple lamp values at once
        # @param [Array] value_array 0 = off, >=1 = on

        # TODO: refactor
        def write(value_array)
          if value_array.is_a?(Hash)
            if value_array.key?(:lamp) && value_array.key?(:value)
              write_to_lamp(value_array[:lamp], value_array[:value])
            else
              fail ArgumentError
            end
          else
            value_array.map! { |x| x.to_i >= 1 ? 255 : 0 }
            @pwm_driver.write(value_array)
          end
        end
      end
    end
  end
end
