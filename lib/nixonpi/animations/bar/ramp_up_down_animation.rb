require_relative '../animation'
require_relative '../easing'
require_relative '../../configurations/settings'
require_relative '../../../dependency'

##
# This animation increments every number by one, the number of turnarounds can be specified
#
module NixonPi
  module Animations
    class RampUpDownAnimation < Animation
      include Easing

      register :ramp_up_down, self

      ##
      # @param [Hash] options
      # * duration - number of turnarounds, default 5
      # * sleep - sleep duration in seconds, default 0.3
      def initialize(options = {})
        super()
        @options = {sleep: 0.3, total: 3.0}.merge(options)
        @driver = get_injected(:in13_tubes)
      end

      def run(start)
        bar = @options[:bar]
        sleep_step = @options[:sleep]
        start = Time.now
        total_time = @options[:total] * 1000.0
        index = 0
        elapsed = 0
        while elapsed < total_time do
          elapsed = time_diff_milli(start, Time.now)
          value = ease_in_out_quad(elapsed, 0, 255, total_time)
          animation_value = value.ceil
          write({port: bar, value: animation_value}, index)
          index += 1
          sleep sleep_step
        end
      end


    end

  end
end
