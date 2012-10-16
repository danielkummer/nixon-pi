module ConversionHelper
  def hex_to_bit(input)
    #string -> hex -> binary
    input.to_s.to_i(16).to_s(2)
  end

  def bit_to_hex(input)
    # string > binary -> hex -> justify right
    input.to_i(2).to_s(16).rjust(2, '0')
  end
end
