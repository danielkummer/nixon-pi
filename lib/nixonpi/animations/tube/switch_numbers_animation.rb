require_relative '../animation'
##
# This animation increments every number by one, the number of turnarounds can be specified
#

# TODO: refactor!!
module NixonPi
  module Animations
    class SwitchNumbersAnimation < Animation
      register :switch_numbers, self
      accepted_commands :start_value, :turnarounds

      ##
      # @param [Hash] options     &
      # * turnarounds - number of turnarounds, default 5
      # * sleep - sleep duration in seconds, default 0.3
      def initialize(options = {})
        super(options)
        turnaround = @options[:turnarounds]
        turnaround ||= 5
        value = @options[:start_value]

        turnaround.times do
          value = value.each_char.collect do |x|
            if x =~ /\d/
              x.to_i + 1 % 10
            else
              x
            end
          end.join
          @output << value
        end
      end

      def write
        handle_output_on_tick(@output.shift)
      end
    end
  end
end
