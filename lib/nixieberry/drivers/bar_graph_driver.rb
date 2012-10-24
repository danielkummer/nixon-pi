require 'singleton'

require_relative '../logging/logging'
require_relative '../configurations/configuration'
require_relative '../../../spec/mocks/mock_abiocardclient'

module NixieBerry
  class BarGraphDriver
    include Logging
    include Configuration
    include Singleton


    def initialize
      #create accessor for each bar
      @client = NixieBerry::AbioCardClient.instance
      #@client = MockAbiocardClient.instance # for testing - must find a better solution here
      @client.pwm_reset
      dim_all(100)
      @bar = {}
      log.info "initialize nixie pwm bars"
    end

    ##
    # Write a percent value to a IN-13 bargraph
    #
    # @param [Integer] bar
    # @param [Integer] percent
    def write_percent(bar, percent)
      @bar[bar.to_i] = percent
      value = (percent / 100.0 * 255.0).round.to_i
      log.debug "write bar #{bar}, #{percent}% value #{value}"
      @client.pwm_write(bar.to_i, value)
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