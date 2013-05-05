require_relative '../animation'

module NixonPi
  module Animations
    class CountFromToAnimation < Animation

      register :count_from_to, self
      accepted_commands :start_value, :single_digit?

      #todo unfinished and untested
      def initialize(options = {})
        super(options)
        @options[:single_digit?] ||= true


        from = @options[:start_value]
        to = from
        if @options[:single_digit?]
          from.each_char { |f| to[i] = (f.to_i += 1).to_s }
          from.reverse.each_char.with_index do |number|
            @output << number
          end
        end
      end

      def write
        handle_output_on_tick(@output.shift)
      end

    end
  end
end
