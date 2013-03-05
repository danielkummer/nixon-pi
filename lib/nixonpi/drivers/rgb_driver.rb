require_relative 'pwm_driver'

module NixonPi
  class RgbDriver
    include Logging

    def initialize(rgb_ports)
      raise "3 ports needed for red green and blue" unless rgb_ports.size == 3
      @io_driver = PwmDriver.new(rgb_ports)
      log.info "Initializing rgb driver...\n red: #{rgb_ports[0]} green: #{rgb_ports[1]} blue: #{rgb_ports[2]}"

    end

    def write_hex(hex_color)
      rgb = hex2int(hex_color)
      write(rgb)
    end

    ##
    # Returns an int rgb value
    # @@param [String] hex_color
    def hex2int(hex_color)
      hex_color = hex_color.gsub('#', '')
      hex_color.scan(/../).map { |color| color.hex }
    end

    def write(rgb_array)
      @io_driver.write(rgb_array)
    end


    def fade(from_hex, to_hex, sleep = 10)
      from_rgb, to_rgb = hex2int(from_hex), hex2int(to_hex)



      from_rgb.each.with_index do |color,i|
        rgb_step[i] = (color - to_rgb[i] / 256 )
      end

      @io_driver.write(from_rgb)

      rgb_step.each.with_index do |color, i|
        from_rgb = from_rgb[i] - color
      end
      sleep(sleep)

=begin
for (int x=0; x<3; x++) {
    INC[x] = (RGB1[x] - RGB2[x]) / 256; }

  for (int x=0; x<256; x++) {

    red = int(RGB1[0]);
    green = int(RGB1[1]);
    blue = int(RGB1[2]);

    analogWrite (RedPin, red);
    analogWrite (GreenPin, green);
    analogWrite (BluePin, blue);
    delay(250);

    for (int x=0; x<3; x++) {
         RGB1[x] -= INC[x];}

  }

  for (int x=0; x<3; x++) {
  RGB2[x] = random(956)-700;
  RGB2[x] = constrain(RGB2[x], 0, 255);

  delay(1000);
 }
=end
    end

  end
end