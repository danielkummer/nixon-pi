require 'singleton'

require_relative '../../../spec/mocks/mock_abiocardclient'

module NixieBerry
  class TubeDriver
    include NixieLogger
    include NixieConfig
    include Singleton

    BLANK_NUM = 10
    #elementOrder 1,6,2,7,5,0,4,9,8,3

    def initialize
      @data_pin, @clock_pin, @latch_pin = config[:tubes][:data_pin], config[:tubes][:clock_pin], config[:tubes][:latch_pin]

      @client = NixieBerry::AbioCardClient.instance
      #@client = MockAbiocardClient.instance # for testing - must find a better solution here
      init_pins

      log.info "initialize nixie with pins - data: #{@data_pin}, clock: #{@clock_pin}, latch: #{@latch_pin}"
    end

    def init_pins
      @client.io_write(@data_pin, 0)
      @client.io_write(@clock_pin, 0)
      @client.io_write(@latch_pin, 0)
    end

    def write(output)
      log.info "write : #{output.to_s} in reverse order"
      @client.io_write(@latch_pin, 0)
      output.split('').reverse.each do |digit|
        serialize_digit(digit)
      end
      @client.io_write(@latch_pin, 1)
      @client.io_write(@latch_pin, 0)
    end

    #Private function that sends a digit out to the shift register
    # writeDigit must be 0 <= x <= 10
    def serialize_digit(digit)
      log.info "serialize digit: #{digit}"
      bitmask = 8

      @client.io_write(@data_pin, 0)

      #send out the bits of the nibble MSB -> LSB
      4.times do
        @client.io_write(@clock_pin, 0)
        current_bit = bitmask & digit.to_i
        current_bit = current_bit == 0 ? 0 : 1
        log.debug "current bit: #{current_bit}"
        @client.io_write(@data_pin, current_bit)
        @client.io_write(@clock_pin, 1)
        bitmask = bitmask >> 1
      end
      @client.io_write(@data_pin, 0)
      @client.io_write(@clock_pin, 0)
    end

    def clear(digits)
      log.info "clear #{digits} digits"
      @client.io_write(@latch_pin, 0)
      digits.times do
        serialize_digit(10)
      end
      @client.io_write(@latch_pin, 1)
      @client.io_write(@latch_pin, 0)
    end

    # Write a full number, right justified, padded with blanks
    def write_trimmed_number(number, digits)
      log.info "write num #{number} trim #{digits}"
      @client.io_write(@latch_pin, 0)

      if number == 0
        serialize_digit(0)
        digits -= 1
      end

      # Write the digits out from right to left
      digits.times do
        number == 0 ? serialize_digit(BLANK_NUM) : serialize_digit(number%10)
        number = number / 10
      end
      @client.io_write(@latch_pin, 1)
      @client.io_write(@latch_pin, 0)
    end

    # Write a full number, right justified, padded with zeros
    def write_zero_padded_number(number, digits)
      log.info "write num #{number} trim #{digits}"
      @client.io_write(@latch_pin, 0)
      #Write the digits out from right to left
      digits.times do
        serialize_digit(number%10)
        number = number / 10
      end
      @client.io_write(@latch_pin, 1)
      @client.io_write(@latch_pin, 0)
    end
  end
end