
module NixonPi
  #
  # This module helps converting hex strings to bits and vice versa
  #
  module HexBitTranslator
    # Convert a hex to a binary string
    # @return [String]
    # @param [String] input
    def hex_to_bit(input)
      # string -> hex -> binary
      input.to_s.to_i(16).to_s(2)
    end

    # Convert a binary string to a right adjusted hex string
    # string -> binary -> hex -> justify right
    # @param [String] input
    # @return [String]
    def bit_to_hex(input)
      input.to_i(2).to_s(16).rjust(2, '0').upcase
    end
  end
end
