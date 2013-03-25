require_relative '../driver'

module NixonPi
  class PwmDriver
    include Driver

    def initialize(options = {ports: nil})
      client.pwm_reset
      dim_all(100)
      @values = {}
      @ports = options[:ports]
      log.debug "initialize pwm ports #{@ports.to_s}"
    end

    def pwm_values
      @values.values
    end

    ##
    # Write a value array to multiple pwm ports (or a single port if the param is a hash)
    # @param [Array] values of integers between 0 and 255
    #
    def write(values)
      if values.kind_of? Hash #todo hacky - refactor
        raise "wrong hash format, expected the keys 'port' and 'value'" unless values.has_key?(:port) and values.has_key?(:value)
        write_to_port(values[:port], values[:value])
      elsif values.kind_of? Array
        log.error "more values than configured lamps" and return if values.size > @ports.size
        values.map! { |x| x.to_i > 255 ? 255 : x.to_i }
        client.pwm_write_registers(start_index: @ports.sort.first, values: values)
      end
    end

    ##
    # Write a value to a pwm port
    #
    # @param [Integer] port
    # @param [Integer] value
    def write_to_port(port, value)
      value = value.to_i
      @values[port.to_i] = value
      log.debug "write bar #{port} value #{value}"
      client.pwm_write(@ports[port.to_i], value)
    end

    ##
    # Dim all bars
    #
    # @param [Integer] percent
    def dim_all(percent)
      value = (percent / 100.0 * 255.0).round.to_i
      log.debug "dim all  #{percent}% value #{value}"
      client.pwm_global_dim(value)
    end
  end
end