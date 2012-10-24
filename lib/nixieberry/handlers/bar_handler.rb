require 'json'
require_relative '../logging/logging'
require_relative '../drivers/bar_graph_driver'
require_relative '../configurations/control'

module NixieBerry
  class BarHandler
    include Logging

    def initialize
      @driver = NixieBerry::BarGraphDriver.instance
      @controlconfig = NixieBerry::Control.instance
    end

    def write_to_bars
      bars = JSON.parse(@controlconfig[:bars])
      bars.each do |bar, value|
        log.debug "write bar #{bar}, value #{value}"
        @driver.write_percent(bar.to_i, value)
      end unless bars.nil?
    end


    def stop_fading
      Thread.kill(@fade_thread)
    end

    def start_fading(cycle_time_in_seconds = 1, times = nil)
      @fade_thread = Thread.new do
        if times
          times.times { self.fade(cycle_time_in_seconds) }
        else
          loop { self.fade(cycle_time_in_seconds) }
        end
      end
    end
  end

  private
  def fade(frequency_in_seconds)
    #null _> high -> null
    self.on
    sleep frequency_in_seconds
    self.off
    sleep frequency_in_seconds
  end


end