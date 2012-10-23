require File.expand_path(File.dirname(__FILE__) + '/retryable.rb')
require File.expand_path(File.dirname(__FILE__) + '/conversion_helper.rb')
require 'net/telnet'
require 'singleton'
require 'thread'


require_relative '../../../spec/mocks/mock_telnet'

module NixieBerry
  class AbioCardClient
    include Retryable
    include ConversionHelper
    include Singleton
    include NixieLogger
    include NixieConfig
    include AnimationQueue

    NUMBER_OF_IO_PORTS = 8
    NUMBER_OF_PWM_PORTS = 16 #number 17 is the global pwm register
    NUMBER_OF_ADC_PORTS = 8

    ##
    # Initialize client connection to telnet server
    def initialize
      @host = config[:telnet_server][:host]
      @port = config[:telnet_server][:port]

      @pwm_register_array = Array.new(NUMBER_OF_PWM_PORTS, 0)

      retryable { connect }
    end

    #TODO
    def read_adc(pin)
      raise NotImplementedError
    end


    ## Write to the rtc clock
    # @param [Time] time
    def clock_write(time)
      time_string = time.strftime("%y%m%d%H%M%S")
      command = ("CW" + time_string)
      handle { @connection.cmd(command) }
    end

    ##
    # Read the rtc clock value
    # @return [Time]
    def clock_read
      clock_regexp = %r{
        ^CR(?<year>\d{2})(?<month>\d{2})(?<day>\d{2})(?<hour>\d{2})(?<minute>\d{2})(?<second>\d{2})(?<powerup>\d)(?<battery>\d)$
      }

      current_time = nil
      handle {
        @connection.cmd("CR") do |return_value|
          r = clock_regexp.match(return_value)
          if r[:powerup] == 0 #if the powerup flag is 1 the time is invalid!
            current_time = Time.new("20" + r[:year], r[:month], r[:day], r[:hour], r[:minute], r[:second])
            @battery_state = r[:battery]
          end
        end
      }
      current_time
    end

    ##
    # Read hardware information
    # @return [Struct] rtc, io, adc, pwm
    def info
      handle {
        @connection.cmd("HI") do |return_value|
          return_value.strip!
          info_bits = hex_to_bit(return_value[3]).split('') #bits 4..7 are reserved, only 0..3 matter
          @hardware_information = Struct.new(:rtc, :io, :adc, :pwm).new(info_bits[3], info_bits[2], info_bits[1], info_bits[0])
          log.info ("read hardware information: " + @hardware_information.to_s)
        end
      }
      @hardware_information
    end

    ##
    # Write IO pin
    # @param [Integer] pin 0..7
    # @param [Integer] value 0..1
    def io_write(pin, value)
      value = 0 unless (0..1).include?(value) # set to 0 if out of bounds

      # write only if the output register changed
      if @out_register.nil? or @out_register[NUMBER_OF_IO_PORTS - 1 - pin] != value.to_s
        @out_register ||= "".rjust(NUMBER_OF_IO_PORTS, '0')

        @out_register[NUMBER_OF_IO_PORTS - 1 - pin] = value.to_s
        handle { @connection.cmd("EW" + bit_to_hex(@out_register)) }
      end
    end

    ##
    # Read IO pin
    # @param [Integer] pin 0..7
    # @return [Integer] pin 0..1
    def io_read(pin)
      handle {
        @connection.cmd("ER") do |return_value|
          return_value.strip!
          @in_register = hex_to_bit(return_value[2..3]).rjust(NUMBER_OF_IO_PORTS, '0')
          log.info ("read return value: " + return_value)
        end
      }
      @in_register[NUMBER_OF_IO_PORTS - 1 - pin].to_i
    end

    ##
    # Write to a pwm register, note that pwm registers are zero based!
    # @param [Integer] number 0..16
    # @param [Integer] value  0.255
    def pwm_write(number, value)
      @pwm_register_array[number] = value
      pwm_write_registers(start_index: 0, values: @pwm_register_array)
    end

    ##
    # Reset all pwm registers to 0
    def pwm_reset
      pwm_write_registers(start_index: 0, values: [0] * NUMBER_OF_PWM_PORTS)
    end

    ##
    # Dim all pwm registers at once over the global pwm register no. 16
    # @param [Integer] value 0..255
    def pwm_global_dim(value)
      @pwm_register_array[NUMBER_OF_PWM_PORTS] = value
      pwm_write_registers(start_index: NUMBER_OF_PWM_PORTS, values: [value])
    end

    ##
    # Write to the 16 pwm registers
    # SS::    Start index, 00..10 hexadecimal.
    # CC::    Count, 01..11 hexadecimal.
    # [XX]::  Array of register values, 00..FF hexadecimal. The count field indicates the number of array elements.
    #	    	  Response
    # CMD::   PWSSCC[XX]
    # Indexes 0 to 15 correspond with registers PWM0 to PWM15, index 16 with register GRPPWM in the LED driver chip.
    def pwm_write_registers(options)
      start_at_register, values = options[:start_index], options[:values]

      start_at_register = 0 unless (0..NUMBER_OF_PWM_PORTS).include?(start_at_register) #set to 0 if out of bounds
      start_at_register = start_at_register.to_s(16).rjust(2, '0') #convert startindex to hex string
      register_count = values.size.to_s(16).rjust(2, '0')

      output_array_string = ""

      #convert values to hex string
      values.each do |value|
        value = 0 unless (0..255).include?(value) #set to 0 if out of bounds
        output_array_string << value.to_s(16).rjust(2, '0')
      end

      command = ("PW" + start_at_register + register_count + output_array_string).upcase
      handle { @connection.cmd(command) }
    end

    protected
    ##
    # Connect to abiocard telnet server
    def connect
      @connection = Net::Telnet::new("Host" => @host, "Port" => @port, "Telnetmode" => false, "Prompt" => //, "Binmode" => true) do |resp|
        log.debug(resp)
      end
      #@telnet = MockTelnet.new
    end

    ##
    # Handle telnet exceptions
    def handle
      begin
        yield
      rescue Exception => exception
        log.error exception.message
        connect || raise
      end
    end

    ##
    # Close telent connection
    def exit
      @connection.close
    end

  end
end