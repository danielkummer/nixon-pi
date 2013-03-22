require_relative 'basic/pwm_driver'
require_relative 'driver'

module NixonPi
  class RgbDriver
    include Logging
    include Driver #todo doesn't need to be a driver

    def initialize(rgb_ports)
      raise "3 ports needed for red green and blue" unless rgb_ports.size == 3
      @io_driver = PwmDriver.new(rgb_ports)
      @rgb_ports = rgb_ports #todo refactor
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
      hex_color = hex_color.to_s
      if hex_color.length < 6
        log.error "wrong color supplied: #{hex_color}, setting to 000000"
        hex_color = "000000"
      end

      rgb = hex2int(hex_color)
      @rgb_ports.each.with_index do |port, i|
        @io_driver.write_to_port(port,rgb[i])
      end
    end

  end
end