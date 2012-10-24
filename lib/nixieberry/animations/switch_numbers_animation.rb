require_relative 'animation'
##
# This animation increments every number by one, the number of turnarounds can be specified
#
module NixieBerry
  module Animations
    class SwitchNumbersAnimation < Animation

      register_animation :switch_numbers

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
        @t = Thread.new do
          duration.times.with_index do |index|
            value = value.each_char.collect { |x| x.to_i + 1 % 10 }.join
            log.debug "write value: #{value}"
            write(value, index)
            sleep sleep_step
          end
          log.debug "write value: #{value}"
          @semaphore.synchronize { @driver.write(start) }
        end
        @t.join
      end

    end
  end
end