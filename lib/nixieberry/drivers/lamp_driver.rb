require 'singleton'
require_relative 'driver'
require_relative '../configurations/settings'

module NixieBerry
  class LampDriver < Driver
    include Logging
    include Singleton

    def initialize
      super()
      @pin_array = Settings.in1_pins
    end

    ##
    # Write to a single lamp
    # @param [Integer] number
    # @param [Integer] value 0 or > 1
    def write_to_lamp(number, value)
      value = value >= 1 ? 255 : 0
      @client.pwm_write(@pin_array[number], value)
    end

    ##
    # Write multiple lamp values at once
    # @param [Array] value_array 0 = off, >=1 = on
    def write(value_array)
      log.error "more values than configured lamps" and return if value_array.size > @pin_array.size
      value_array.map!{|x| x >= 1 ? 255 : 0 }
      @client.pwm_write_registers(start_index: @pin_array.first, values: value_array)
    end
  end
end