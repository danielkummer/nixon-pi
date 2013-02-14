require_relative 'driver'
require_relative '../configurations/settings'

module NixonPi
  class IODriver
    include Logging
    include Driver

    def initialize(*pins)
      #noinspection RubyResolve
      @ports = Set.new
      @ports << pins
      #@ports = Settings.in1_pins
      log.info "Initializing lamps with pins: #{@ports.to_s}"
    end

    ##
    # Write to a single lamp
    # @param [Integer] number
    # @param [Integer] value 0 or > 1
    def write_to_port(number, value)
      value = value.to_i >= 1 ? 255 : 0
      client.pwm_write(@ports[number], value)
    end

    ##
    # Write multiple lamp values at once
    # @param [Array] value_array 0 = off, >=1 = on
    def write(value_array)
      log.error "more values than configured lamps" and return if value_array.size > @ports.size
      value_array.map!{|x| x.to_i >= 1 ? 255 : 0 }
      client.pwm_write_registers(start_index: @ports.first, values: value_array)
    end
  end
end