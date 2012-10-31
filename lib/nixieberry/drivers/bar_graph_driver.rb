require 'singleton'

require_relative '../logging/logging'
require_relative '../configurations/settings'
require_relative '../../../spec/mocks/mock_abiocardclient'

module NixieBerry
  class BarGraphDriver
    include Logging
    include Singleton

    def initialize
      #create accessor for each bar
      @client = NixieBerry::AbioCardClient.instance
      #@client = MockAbiocardClient.instance # for testing - must find a better solution here
      @client.pwm_reset
      dim_all(100)
      @bar_values = {}
      @pin_array = Settings.in13_pins
      log.info "initialize nixie pwm bars"
    end

    def bar_values
      @bar_values.values
    end

    ##
    # Write a value array to multiple bargraphs
    #
    def write(value_array)
      log.error "more values than configured lamps" and return if value_array.size > number_of_bars
      value_array.map! { |x| x > 255 ? 255 : x }
      @client.pwm_write_registers(start_index: @pin_array.first, values: value_array)
    end

    ##
    # Write a percent value to a IN-13 bargraph
    #
    # @param [Integer] bar
    # @param [Integer] percent
    def write_percent(bar, percent)
      @bar_values[bar.to_i] = percent
      value = (percent / 100.0 * 255.0).round.to_i
      log.debug "write bar #{bar}, #{percent}% value #{value}"
      @client.pwm_write(@pin_array[bar.to_i], value)
    end

    ##
    # Dim all bars
    #
    # @param [Integer] percent
    def dim_all(percent)
      value = (percent / 100.0 * 255.0).round.to_i
      log.debug "dim all  #{percent}% value #{value}"
      @client.pwm_global_dim(value)
    end

  end
end