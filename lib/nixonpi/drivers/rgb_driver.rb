require_relative 'pwm_driver'

module NixonPi
  class RgbDriver
    include Logging

    def initialize(rgb_ports)
      raise "3 ports needed for red green and blue" unless rgb_ports.size == 3
      @io_driver = PwmDriver.new(rgb_ports)
      log.info "Initializing rgb driver...\n red: #{rgb_ports[0]} green: #{rgb_ports[1]} blue: #{rgb_ports[2]}"

    end

    ##
    # Returns an int rgb value
    # @@param [String] hex_color
    def hex2int(hex_color)
      hex_color = hex_color.gsub('#', '')
      hex_color.scan(/../).map { |color| color.hex }
    end

    def write(hex_color)
      rgb = hex2int(hex_color)
      @io_driver.write(rgb)
    end

  end
end