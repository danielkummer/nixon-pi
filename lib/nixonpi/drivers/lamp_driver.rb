require_relative '../configurations/settings'
require_relative 'basic/pwm_driver'
require_relative 'driver'

module NixonPi
  class LampDriver
    include Logging
    include Driver #todo doesn't need to be a driver

    def initialize(ports)
      log.info "Initializing lamps driver"
      @pwm_driver = PwmDriver.new(ports)
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
    def write(value_array)
      log.error "more values than configured lamps" and return if value_array.size > @ports.size
      value_array.map!{|x| x.to_i >= 1 ? 255 : 0 }
      @pwm_driver.write(value_array)
    end
  end
end