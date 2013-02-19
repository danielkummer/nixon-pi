require_relative 'pwm_driver'

module NixonPi
  class RgbDriver
    include Logging

    def initialize(rgb_ports)
      raise "3 ports needed for red green and blue" unless rgb_ports.size == 3
      @io_driver = PwmDriver.new(rgb_ports)
      log.info "Initializing rgb driver..."
    end

    def write_hex(color)
      #hex 2 int
    end

    def write(rbg_array)

    end





    def fadeHex(hex, hex2, ratio)
      #http://stackoverflow.com/questions/84421/converting-an-integer-to-a-hexadecimal-string-in-ruby
      r = hex >> 16
      g = hex >> 8 & 0xFF
      b = hex & 0xFF
      r += ((hex2 >> 16)-r)*ratio
      g += ((hex2 >> 8 & 0xFF)-g)*ratio
      b += ((hex2 & 0xFF)-b)*ratio
      return(r<<16 | g<<8 | b)
    end


  end
end