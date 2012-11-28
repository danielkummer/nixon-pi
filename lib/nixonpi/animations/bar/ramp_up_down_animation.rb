require_relative '../animation'
require_relative '../easing'
require_relative '../../configurations/settings'

##
# This animation increments every number by one, the number of turnarounds can be specified
#
module NixonPi
  module Animations
    class RampUpDownAnimation < Animation
      include Easing

      register_as :ramp_up_down

      ##
      # @param [Hash] options
      # * duration - number of turnarounds, default 5
      # * sleep - sleep duration in seconds, default 0.3
      def initialize(options = {})
        super()
        @options = {sleep: 0.3, total: 3.0}.merge(options)
        @driver = BarGraphDriver.instance
      end

      def run(start)
        number_of_tubes = Settings.in13_pins.size
        sleep_step = @options[:sleep]
        animation_values = Array.new(number_of_tubes, 0)
        start = Time.now
        total_time = @options[:total] * 1000.0
        index = 0
        elapsed = 0
        while elapsed < total_time do
          elapsed = time_diff_milli(start, Time.now)
          value = ease_in_out_quad(elapsed, 0, 255, total_time)

          animation_values = [value] * number_of_tubes
          animation_values.map! { |value| value = value.ceil}

          log.debug "write value: #{animation_values}"
          write(animation_values, index)
          index += 1
          sleep sleep_step
        end
      end


    end

  end
end
