require File.expand_path(File.dirname(__FILE__) + '/conversion_helper.rb')
require 'net/telnet'
require 'singleton'
require 'thread'


require_relative '../../test/mocks/mock_telnet'

module NixieBerry
  class AbioCardClient
    include ConversionHelper
    include Singleton
    include NixieLogger
    include NixieConfig
    include AnimationQueue


    NUMBER_OF_IO_PORTS = 8


    # If retry times more than retry times in option parameter, will raise a RetryError.
    # * :retry_times - Retry times , Defaults 10
    # * :on - The Exception on which a retry will be performed. Defaults Exception
    # Notice: This method will call block many times, so don't put can't retryable code in it.
    # Example
    # =======
    #    begin
    #      retryable_proxy(:retry_times => 10,:on => Timeout::Error) do |ip,port|
    #         # your code here
    #      end
    #    rescue RetryError
    #      # handle error
    #    end
    #
    def retryable(options = {})
      opts = {:retry_times => 10, :on => Exception}.merge(options)
      retry_times, try_exception = opts[:retry_times], opts[:on]
      begin
        yield
      rescue try_exception => e
        if (retry_times -= 1) > 0
          log.info "retrying another #{retry_times} times"
          retry
        end
        log.info "failed retrying... #{e.message}"
        raise Exceptions::RetryError
      end
    end


    def handle
      begin
        @semaphore.synchronize {
          yield
        }
      rescue Exception => exception
        log.error exception.message
        connect || raise
      end
    end


    def initialize
      @host = config[:telnet_server][:host]
      @port = config[:telnet_server][:port]
      @pwm_registers = Array.new

      @semaphore = Mutex.new


      16.times do |i|
        @pwm_registers[i] = 0
      end

      retryable { connect }

      initialize_animation_thread
    end

    def initialize_animation_thread
      @animationThread = Thread.new do
        loop do #until queue.empty?
          animation = queue.pop(true) rescue nil
          if animation
            case animation[:type]
              when :pwm
                AbioCardClient.instance.write_pwm(animation[:port], animation[:value])
              when :io
                AbioCardClient.instance.write_io_pin(animation[:port], animation[:value])
              else
                raise "animation not defined"
            end
            sleep animation[:sleep] if animation[:sleep]
          end
        end
      end
    end

    def connect
      @telnet = Net::Telnet::new("Host" => @host, "Port" => @port, "Telnetmode" => false, "Prompt" => //, "Binmode" => true) do |resp|
        log.debug(resp)
      end
      #@telnet = MockTelnet.new
    end

    def exit
      #@telnet.write("QU") # server shutdown, stops the server process!!
      @telnet.close
    end


    def write_io_pin(pin_number, value)
      value = 0 unless value == 0 || value == 1
      if @out.nil? or @out[NUMBER_OF_IO_PORTS - 1 - (pin_number - 1)] != value.to_s
        @out ||= "".rjust(NUMBER_OF_IO_PORTS, '0')

        @out[NUMBER_OF_IO_PORTS - 1 - (pin_number - 1)] = value.to_s
        handle { @telnet.cmd("EW" + bit_to_hex(@out)) }
      end
      true
    end

    def read_io_pin(pin_number)
      handle {
        @telnet.cmd("ER") { |ret|
          ret.strip!
          @in = hex_to_bit(ret[2..3]).rjust(8, '0')
          log.info ("read return value: " + ret)
        }
      }
      @in[7 - (pin_number -1)].to_i
    end

    #pwm registers are zero based!
    def write_pwm(number, value)
      @pwm_registers[number] = value
      write_pwm_registers(start_index: 0, values: @pwm_registers)
    end

    def reset_pwm
      write_pwm_registers(start_index: 0, values: [0] * 16)
    end

    def dim_global_pwm(value)
      @pwm_registers[16] = value
      write_pwm_registers(start_index: 16, values: [value])
    end

    #SS		Start index, 00..10 hexadecimal.
    #CC		Count, 01..11 hexadecimal.
    #[XX]	Array of register values, 00..FF hexadecimal. The count field indicates the number of array elements.
    #		  Response
    #CMD  PWSSCC[XX]
    #Indexes 0 to 15 correspond with registers PWM0 to PWM15, index 16 with register GRPPWM in the LED driver chip.
    def write_pwm_registers(options)
      start_index, values = options[:start_index], options[:values]

      start_index = 0 unless (0..16).include?(start_index)
      start_index = start_index.to_s(16).rjust(2, '0')

      val_array_string = ""
      values.each do |value|
        value = 0 unless (0..255).include?(value)
        val_array_string << value.to_s(16).rjust(2, '0')
      end
      command = ("PW" + start_index + values.size.to_s(16).rjust(2, '0') + val_array_string).upcase
      handle { @telnet.cmd(command) }
    end
  end
end

=begin
 def start_server_daemon
      unless @pid.nil?
        begin
          Process.getpgid(@pid)
        rescue Errno::ESRCH
          log.error "unable to end process with pid: " + @pid.to_s
        end
      end
      log.info "starting abiocardserver"
      if system "/opt/abiocard/abiocardserver -p 5678 &"
        @pid = $?.pid
      else
        log.error "abiocard service not statet"
      end
    end
=end