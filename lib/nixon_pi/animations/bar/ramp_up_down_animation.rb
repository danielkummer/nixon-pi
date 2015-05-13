
module NixonPi
  module Animations
    class RampUpDownAnimation < Animation
      include Easing

      register :ramp_up_down, self
      accepted_commands :bar, :total_time

      ##
      # @param [Hash] options
      # * duration - number of turnarounds, default 5
      # * sleep - sleep duration in seconds, default 0.3
      def initialize(options = {})
        super(options)
        @options[:total_time] ||= 3.0
        @options[:sleep] ||= 0.3
        @total_time = @options[:total_time] * 1000.0
        @start = Time.now
        @elapsed = 0
      end

      def write
        if @elapsed < @total_time
          handle_output_on_tick(port: @options[:bar], value: get_current_value)
        else
          handle_output_on_tick(nil) # exit
        end
      end

      def get_current_value
        @elapsed = time_diff_milli(@start, Time.now)
        ease_in_out_quad(@elapsed, 0, 255, @total_time).ceil
      end
    end
  end
end
