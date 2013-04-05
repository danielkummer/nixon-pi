require_relative '../animation'
##
# This animation lets numbers fly in from left to right, one number at a time...
# Here an example:
# _123 -> 3___ -> _3__ -> __3_ -> ___3 -> 2__3 -> _2_3 -> __23 -> 1_23 -> _123
#
module NixonPi
  module Animations
    class BlinkAnimation < Animation

      register :blink, self


      def initialize(options)
        @options = {times: 5}
        @options.merge!(options)
        @output = Array.new
        @times = @options[:times]
        @value = 1
      end


      def write
        if @times >= 0
          wrapped_write({lamp: @options[:lamp], value:@value})
          @value = @value == 1 ? 0 : 1
          @times = @times - 1
        else
          wrapped_write(nil)
        end
      end

    end
  end
end
