require 'net/telnet'
require 'singleton'
require 'thread'
#require 'profile'

require_relative 'retryable'
require_relative 'conversion_helper'
require_relative '../logging/logging'
require_relative '../configurations/settings'

require_relative '../../../spec/support/mock_telnet'

module NixieBerry
  class AbioCardClient
    include Retryable
    include ConversionHelper
    include Singleton
    include Logging

    NUMBER_OF_IO_PORTS = 8
    NUMBER_OF_PWM_PORTS = 16 #number 17 is the global pwm register
    NUMBER_OF_ADC_PORTS = 8

    ##
    # Initialize client connection to telnet server
    def initialize
      @host = Settings.telnet_server.host
      @port = Settings.telnet_server.port

      @pwm_register_array = Array.new(NUMBER_OF_PWM_PORTS, 0)
      @io_register = Array.new(NUMBER_OF_IO_PORTS, 0)

      @mutex = Mutex.new
      #load_last_values currently not used
    end


    def read_adc(pin)
      #TODO
      raise NotImplementedError
    end


    ## Write to the rtc clock
    # @param [Time] time
    def clock_write(time)
      time_string = time.strftime("%y%m%d%H%M%S")
      command = ("CW" + time_string)
      handle { connection.cmd(command) }
    end

    ##
    # Read the rtc clock value
    # @return [Time]
    def clock_read
      clock_regexp = %r{CR(?<year>\d{2})(?<month>\d{2})(?<day>\d{2})(?<hour>\d{2})(?<minute>\d{2})(?<second>\d{2})(?<powerup>\d)(?<battery>\d)}

      current_time = nil
      handle {
        connection.cmd("CR") do |return_value|
          r = clock_regexp.match(return_value)
          if r[:powerup] == "0" #if the powerup flag is 1 the time is invalid!
            current_time = Time.new(("20" + r[:year]).to_i, r[:month].to_i, r[:day].to_i, r[:hour].to_i, r[:minute].to_i, r[:second].to_i)
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
      #handle {
      connection.cmd("String" => "HI", "Match" => /HI.*/) do |return_value|
        return_value.strip!
        info_bits = hex_to_bit(return_value[3]).to_s.split('') #bits 4..7 are reserved, only 0..3 matter
        @hardware_information = Struct.new(:rtc, :io, :adc, :pwm).new(info_bits[3], info_bits[2], info_bits[1], info_bits[0])
        log.info ("read hardware information: " + @hardware_information.to_s)
      end
      #}
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
        handle { connection.cmd("EW" + bit_to_hex(@out_register)) }
      end
    end

    ##
    # Read IO pin
    # @param [Integer] pin 0..7
    # @return [Integer] pin 0..1
    def io_read(pin)
      handle {
        connection.cmd("ER") do |return_value|
          return_value.strip!
          @io_register = hex_to_bit(return_value[2..3]).rjust(NUMBER_OF_IO_PORTS, '0')
          log.info ("read return value: " + return_value)
        end
      }
      @io_register[NUMBER_OF_IO_PORTS - 1 - pin].to_i
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
    # Read from the 17 pwm registers
    # Command:  PRSSCC<EOL>
    #           SS Start index, 00..10 hexadecimal.
    #           CC Count, 01..11 hexadecimal.
    # Response: PRSSCC[XX]<LF>
    #           SS Start index, 00..10 hexadecimal.
    #           CC Count, 01..11 hexadecimal.
    #           [XX] Array of register values, 00..FF hexadecimal. The count field indicates the number of array elements
    def pwm_read_registers(options = {start_index: 0, count: NUMBER_OF_PWM_PORTS})
      start_at_register, count = options[:start_index], options[:count]
      start_at_register = 0 unless (0..NUMBER_OF_PWM_PORTS).include?(start_at_register) #set to 0 if out of bounds
      start_at_register = start_at_register.to_s(16).rjust(2, '0') #convert startindex to hex string
      register_count = count.to_s(16).rjust(2, '0')
      handle {
        connection.cmd("PR" + start_at_register + register_count) do |return_value|
          return_value.strip!
          log.info ("read pwm return value: " + return_value)
          index = return_value[2..3].to_i(16)
          count = return_value[4..5].to_i(16)
          register_array = return_value[6..-1].scan(/.{2}/).map { |x| x.to_i(16) } #split in pairs of two

          (count - 1).times.with_index do |i|
            @pwm_register_array[index] = register_array[i]
            index += 1
          end
        end
      }
      @pwm_register_array
    end

    ##
    # Write to the 16 pwm registers
    # SS::    Start index, 00..10 hexadecimal.
    # CC::    Count, 01..11 hexadecimal.
    # [XX]::  Array of register values, 00..FF hexadecimal. The count field indicates the number of array elements.
    #	    	  Response
    # CMD::   PWSSCC[XX]
    # Indexes 0 to 15 correspond with registers PWM0 to PWM15, index 16 with register GRPPWM in the LED driver chip.
    # @param [Hash] options, :start_index 0..15, :values 0..255
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
      handle { connection.cmd(command) }
    end

    protected
    def load_last_values
      log.debug "loading last values"
      pwm_read_registers
      io_read(0)
    end

    ##
    # Connect to abiocard telnet server
    def connection
      #todo test sync
      @mutex.synchronize {
        retryable do
          @conn ||= connection_for(ENV['NIXIE_BERRY_ENVIRONMENT'])
        end
      @conn
      }
    end

    def connection_for(env)
      case env.to_sym
        when :development, :production
          Net::Telnet::new("Host" => @host, "Port" => @port, "Telnetmode" => false, "Prompt" => //, "Binmode" => true) do |resp|
            log.debug(resp)
          end
        when :test
          MockTelnet.new
      end
    end

    ##
    # Handle telnet exceptions
    def handle
      begin
        yield
          #rescue Exception => exception
      rescue TimeoutError => exception
        log.error exception.message
        connection || raise
      rescue Exception => e
        raise e
      end
    end

    ##
    # Close telent connection
    def exit
      connection.close
    end

  end
end