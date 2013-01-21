require_relative 'driver'

module NixonPi
  class BarGraphDriver
    include Singleton
    include Driver

    def initialize
      client.pwm_reset
      dim_all(100)
      @bar_values = {}
      @pin_array = Settings.in13_pins
      log.debug "initialize nixie pwm bars #{@pin_array.to_s}"
    end

    def bar_values
      @bar_values.values
    end

    ##
    # Write a value array to multiple bargraphs
    #
    def write(value_array)
      if value_array.kind_of? Hash #todo hacky - refactor
        write_to_bar(value_array[:bar], value_array[:value])
      elsif value_array.kind_of? Array
        log.error "more values than configured lamps" and return if value_array.size > @pin_array.size
        value_array.map! { |x| x.to_i > 255 ? 255 : x.to_i }
        client.pwm_write_registers(start_index: @pin_array.first, values: value_array)
      end
    end

    ##
    # Write a percent value to a IN-13 bargraph
    #
    # @param [Integer] bar
    # @param [Integer] value
    def write_to_bar(bar, value)
      @bar_values[bar.to_i] = value
      log.debug "write bar #{bar} value #{value}"
      client.pwm_write(@pin_array[bar.to_i], value)
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