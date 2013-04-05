require_relative '../animation'

module NixonPi
  module Animations
    class CountFromToAnimation < Animation

      register :count_from_to, self

      #todo unfinished and untested
      def initialize(options = {})
        @options = {single_digit: true}
        @output = Array.new

        from = options[:start_value]
        to = from
        if @options[:single_digit]
          from.each_char { |f| to[i] = (f.to_i += 1).to_s }
          from.reverse.each_char.with_index do |number|
            @output << number
          end
        end
      end

      def write
        wrapped_write(@output.shift)
      end

    end
  end
end
