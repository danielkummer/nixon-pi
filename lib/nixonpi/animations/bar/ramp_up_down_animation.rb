require_relative '../animation'
require_relative '../easing'
require_relative '../../configurations/settings'
require_relative '../../../dependency'

module NixonPi
  module Animations
    class RampUpDownAnimation < Animation
      include Easing

      register :ramp_up_down, self

      ##
      # @param [Hash] options
      # * duration - number of turnarounds, default 5
      # * sleep - sleep duration in seconds, default 0.3
      def initialize(options)

        @options = {sleep: 0.3, total: 3.0}.merge(options)

        @bar = @options[:bar]
        @start = Time.now
        @total_time = @options[:total] * 1000.0
        @elapsed = 0

      end

      def write()
        if @elapsed < @total_time
          wrapped_write({port: @bar, value: get_current_value})
        else
          wrapped_write(nil) #exit
        end
      end

      def get_current_value
        @elapsed = time_diff_milli(@start, Time.now)
        ease_in_out_quad(@elapsed, 0, 255, @total_time).ceil
      end
    end

  end
end
