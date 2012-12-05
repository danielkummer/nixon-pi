require_relative '../animation'
##
# This animation lets numbers fly in from left to right, one number at a time...
# Here an example:
# _123 -> 3___ -> _3__ -> __3_ -> ___3 -> 2__3 -> _2_3 -> __23 -> 1_23 -> _123
#
module NixonPi
  module Animations
    class CountFromToAnimation < Animation

      register_as :count_from_to

      def initialize(options = {sleep: 0.3, single_digit: true})
        @options = options
        super()
      end

      #TODO this can surely be refactored
      def run(start)
        sleep_step = @options[:sleep]
        from = start
        to = from
        if @options[:single_digit])
        from.each_char_with_index { |f,i| to[i] = (f = f.to_i += 1).to_s  }


        from.reverse.each_char.with_index do |number, index|
          write(current_output, index)
          sleep sleep_step
        end
      end

    end
  end
end
