##
# This animation lets numbers fly in from left to right, one number at a time...
# Here an example:
# _123 -> 3___ -> _3__ -> __3_ -> ___3 -> 2__3 -> _2_3 -> __23 -> 1_23 -> _123
#
module NixonPi
  module Animations
    class BlinkAnimation < Animation
      register :blink, self
      accepted_commands :lamp, :no_of_times

      def initialize(options)
        super(options)
        @options[:no_of_times] ||= 5
        @no_of_times = @options[:no_of_times]
        @value = 1
      end

      def write
        if @no_of_times >= 0
          handle_output_on_tick(lamp: @options[:lamp], value: @value)
          @value = @value == 1 ? 0 : 1
          @no_of_times -= 1
        else
          handle_output_on_tick(nil)
        end
      end
    end
  end
end
