require_relative '../animation'
##
# This animation increments every number by one, the number of turnarounds can be specified
#
module NixonPi
  module Animations
    class SwitchNumbersAnimation < Animation

      register_as :switch_numbers

      ##
      # @param [Hash] options
      # * duration - number of turnarounds, default 5
      # * sleep - sleep duration in seconds, default 0.3
      def initialize(options = {})
        @options = {duration: 5, sleep: 0.3}.merge(options)
        super()
      end

      def run(start)
        duration = @options[:duration]
        sleep_step = @options[:sleep]
        value = start
        duration.times.with_index do |index|
          value = value.each_char.collect do |x|
            if x =~ /\d/
              x.to_i + 1 % 10
            else
              x
            end
          end.join
          log.debug "write value: #{value}"
          write(value, index)
          sleep sleep_step
        end
        log.debug "write value: #{value}"
        write(start, duration + 1 )
      end
    end
  end
end