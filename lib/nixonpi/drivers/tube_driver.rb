require 'singleton'

require_relative 'driver'
require_relative '../configurations/settings'
require_relative '../client/abio_card_client'

module NixonPi
  class TubeDriver
    include Singleton
    include Driver

    BLANK_NUM = 10
    #elementOrder 1,6,2,7,5,0,4,9,8,3

    def initialize
      init_pins
    end

    def write_string_with_blanks(output)
      #log.debug "write number with blanks '#{output}'"
      client.io_write(@latch_pin, 0)

      output.split('').reverse.each do |digit|
        digit =~ /\s|_/ ? serialize_digit(BLANK_NUM) : serialize_digit(digit)
      end
      client.io_write(@latch_pin, 1)
      client.io_write(@latch_pin, 0)
    end

    ##
    # Write number
    # @param [String] output
    def write(output)

      if #noinspection RubyResolve
      Settings.in12a_tubes['write_blanks']
        write_string_with_blanks(output)
      else
        write_number_string_with_zeros(output)
      end
    end

    #todo refactor into helper
    def has_canged?(value)
      result = (value == @old_value ? false : true)
      @old_value = value
      result
    end

    def write_number_string_with_zeros(output)
      log.info "write : #{output.to_s} in reverse order"
      client.io_write(@latch_pin, 0)
      output.split('').reverse.each do |digit|
        serialize_digit(digit)
      end
      client.io_write(@latch_pin, 1)
      client.io_write(@latch_pin, 0)
    end

    ##
    # Clear a number of digits
    # @param [Integer] digits
    def clear(digits)
      log.info "clear #{digits} digits"
      client.io_write(@latch_pin, 0)
      digits.times do
        serialize_digit(10)
      end
      client.io_write(@latch_pin, 1)
      client.io_write(@latch_pin, 0)
    end

    ##
    # Write a full number, right justified, padded with blanks
    # @param [Integer] number
    # @param [Integer] digits
    def write_trimmed_number(number, digits)
      log.info "write num #{number} trim #{digits}"
      client.io_write(@latch_pin, 0)

      if number == 0
        serialize_digit(0)
        digits -= 1
      end

      # Write the digits out from right to left
      digits.times do
        number == 0 ? serialize_digit(BLANK_NUM) : serialize_digit(number%10)
        number = number / 10
      end
      client.io_write(@latch_pin, 1)
      client.io_write(@latch_pin, 0)
    end

    ##
    # Write a full number, right justified, padded with zeros
    # @param [Integer] number
    # @param [Integer] digits
    def write_zero_padded_number(number, digits)
      log.info "write num #{number} trim #{digits}"
      client.io_write(@latch_pin, 0)
      #Write the digits out from right to left
      digits.times do
        serialize_digit(number%10)
        number = number / 10
      end
      client.io_write(@latch_pin, 1)
      client.io_write(@latch_pin, 0)
    end

    private

    ##
    # Initialize the shift register control pins
    def init_pins
      #noinspection RubyResolve,RubyResolve,RubyResolve
      @data_pin, @clock_pin, @latch_pin = Settings.in12a_tubes.data_pin, Settings.in12a_tubes.clock_pin, Settings.in12a_tubes.latch_pin
      log.info "initialize nixie with pins - data: #@data_pin, clock: #@clock_pin, latch: #{@latch_pin}"
      client.io_write(@data_pin, 0)
      client.io_write(@clock_pin, 0)
      client.io_write(@latch_pin, 0)
    end

    ##
    # Sends a digit out to the shift register
    # writeDigit must be 0 <= x <= 10
    # @param [Integer] digit
    def serialize_digit(digit)
      bitmask = 8
      client.io_write(@data_pin, 0)
      #send out the bits of the nibble MSB -> LSB
      4.times do
        client.io_write(@clock_pin, 0)
        current_bit = bitmask & digit.to_i
        current_bit = current_bit == 0 ? 0 : 1
        client.io_write(@data_pin, current_bit)
        client.io_write(@clock_pin, 1)
        bitmask = bitmask >> 1
      end
      client.io_write(@data_pin, 0)
      client.io_write(@clock_pin, 0)
    end
  end
end