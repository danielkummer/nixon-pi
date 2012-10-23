require 'singleton'

require_relative '../../../spec/mocks/mock_abiocardclient'

module NixieBerry
  class BarDriver
    include NixieLogger
    include NixieConfig
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

    #bar is zero based!!
    def write_percent(bar, percent)
      @bar[bar.to_i] = percent
      value = (percent / 100.0 * 255.0).round.to_i
      log.debug "write bar #{bar}, #{percent}% value #{value}"
      @client.pwm_write(bar.to_i, value)
    end

    def dim_all(percent)
      value = (percent / 100.0 * 255.0).round.to_i
      log.debug "dim all  #{percent}% value #{value}"
      @client.pwm_global_dim(value)
    end

  end
end