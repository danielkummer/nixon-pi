#
# Help with string format conversions
#
module NixonPi
  module ConversionHelper

    # Convert a hex to a binary string
    # @return [String]
    # @param [String] input
    def hex_to_bit(input)
      #string -> hex -> binary
      input.to_s.to_i(16).to_s(2)
    end

    # Convert a binary string to a right adjusted hex string
    # @param [String] input
    # @return [String]
    def bit_to_hex(input)
      # string > binary -> hex -> justify right
      input.to_i(2).to_s(16).rjust(2, '0')
    end
  end
end
