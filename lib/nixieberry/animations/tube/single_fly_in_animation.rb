require_relative '../animation'
##
# This animation lets numbers fly in from left to right, one number at a time...
# Here an example:
# _123 -> 3___ -> _3__ -> __3_ -> ___3 -> 2__3 -> _2_3 -> __23 -> 1_23 -> _123
#
module NixieBerry
  module Animations
    class SingleFlyInAnimation < Animation

      register_animation :single_fly_in

      def initialize(options = {})
        @options = {sleep: 0.3}.merge(options)
        super()
      end

      #TODO this can surely be refactored
      def run(start)
        sleep_step = @options[:sleep]
        value = start

        @thread = Thread.new do
          original_length = value.length
          pad_times = original_length
          first_number_position = value.index(/\d/)
          last_output_of_number = ""
          append_number = ""

          value.reverse.each_char.with_index do |number, index|
            pad_times.times do |current|
              current_output = ""
              current_output << "_" * (current)
              current_output << number.to_s
              current_output << "_" * (original_length - current - append_number.length - 1)
              current_output << append_number
              last_output_of_number = current_output
              write(current_output, index)
              sleep sleep_step
            end
            append_number = last_output_of_number[pad_times - 1] + append_number
            pad_times = pad_times - 1
            break if pad_times == first_number_position
          end
        end

        @thread.join
      end
    end
  end
end
