require_relative '../animation'
##
# This animation lets numbers fly in from left to right, one number at a time...
# Here an example:
# _123 -> 3___ -> _3__ -> __3_ -> ___3 -> 2__3 -> _2_3 -> __23 -> 1_23 -> _123
#
module NixonPi
  module Animations
    class SingleFlyInAnimation < Animation
      include Commands

      register :single_fly_in, self
      accepted_commands :start_value, :goto_state, :goto_target


      def initialize(options = {})
        super(options)

        value = @options[:start_value]
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
            @output << current_output
          end
          append_number = last_output_of_number[pad_times - 1] + append_number
          pad_times = pad_times - 1
          break if pad_times == first_number_position
        end


      end

      def write
        handle_output_on_tick(@output.shift)
      end

    end
  end
end
