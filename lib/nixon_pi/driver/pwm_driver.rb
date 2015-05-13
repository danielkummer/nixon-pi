
module NixonPi
  module Driver
    class PwmDriver
      include Driver
      include Logging

      def initialize(options = {ports: nil})
        client.pwm_reset
        dim_all(100)
        @values = {}
        @ports = options[:ports]
        log.debug "initialize pwm ports #{@ports}"
      end

      def pwm_values
        @values.values
      end

      ##
      # Write a value array to multiple pwm ports (or a single port if the param is a hash)
      # @param [Array] values of integers between 0 and 255
      #
      def write(values)
        if values.is_a? Hash # TODO: hacky - refactor
          fail "wrong hash format, expected the keys 'port' and 'value'" unless values.key?(:port) && values.key?(:value)
          write_to_port(values[:port], values[:value])
        elsif values.is_a? Array
          log.error('more values than configured lamps') && return if values.size > @ports.size
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
        # log.debug "write bar #{port} value #{value}"
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
end
