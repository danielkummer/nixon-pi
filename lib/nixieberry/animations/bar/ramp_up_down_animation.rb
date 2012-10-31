require_relative '../animation'
##
# This animation increments every number by one, the number of turnarounds can be specified
#
module NixieBerry
  module Animations
    class RampUpDownAnimation < Animation

      register_animation :ramp_up_down

      ##
      # @param [Hash] options
      # * duration - number of turnarounds, default 5
      # * sleep - sleep duration in seconds, default 0.3
      def initialize(options = {})
        super()
        @options = {sleep: 0.3}.merge(options)
        @driver = BarGraphDriver.instance
      end

      def run()
        initial_values = @driver.bar_values
        number_of_tubes = initial_values.size
        sleep_step = @options[:sleep]
        animation_values = Array.new(number_of_tubes, 0)
        @t = Thread.new do

            #todo ramp values up and down for a given duration
            log.debug "write value: #{animation_values}"
            write(value, index)
            sleep sleep_step
          end
          log.debug "write value: #{animation_values}"
          write(initial_values, index)
        end
        @t.join
      end

    end
  end
end