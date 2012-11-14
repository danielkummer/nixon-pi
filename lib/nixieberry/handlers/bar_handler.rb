require 'json'
require_relative '../logging/logging'
require_relative '../drivers/bar_graph_driver'
require_relative '../configurations/state_hash'
require_relative '../command_queue'

# @Deprecated
#Todo completely outdated
#Todo add statemachine
#Todo add controlconfig inizialization

module NixieBerry
  class BarHandler
    include Logging
    include CommandQueue

    def initialize
      @driver = NixieBerry::BarGraphDriver.instance

      #@controlconfig[:bars] = {mode: nil, free_value: nil, animation_name: nil, animation_options: nil}


    end

    def write_to_bars
      bars = JSON.parse(@controlconfig[:bars])
      bars.each do |bar, value|
        log.debug "write bar #{bar}, value #{value}"
        @driver.write_percent(bar.to_i, value)
      end unless bars.nil?
    end

  end

end