module NixonPi
  module Animations
    class RgbAnimation < Animation
      include NixonPi::DependencyInjection

      register :rgb_animation, self
      accepted_commands :mode, :colors

      # ##todo

      def initialize(options = {})
        super(options)
      end

      # todo

      def fade(from_hex, to_hex, sleep = 10)
        from_rgb = hex2int(from_hex)
        to_rgb = hex2int(to_hex)

        from_rgb.each.with_index do |color, i|
          rgb_step[i] = (color - to_rgb[i] / 256)
        end

        @io_driver.write(from_rgb)

        rgb_step.each.with_index do |color, i|
          from_rgb = from_rgb[i] - color
        end
        sleep(sleep)

        #       for (int x=0; x<3; x++) {
        #           INC[x] = (RGB1[x] - RGB2[x]) / 256; }
        #
        #         for (int x=0; x<256; x++) {
        #
        #           red = int(RGB1[0]);
        #           green = int(RGB1[1]);
        #           blue = int(RGB1[2]);
        #
        #           analogWrite (RedPin, red);
        #           analogWrite (GreenPin, green);
        #           analogWrite (BluePin, blue);
        #           delay(250);
        #
        #           for (int x=0; x<3; x++) {
        #                RGB1[x] -= INC[x];}
        #
        #         }
        #
        #         for (int x=0; x<3; x++) {
        #         RGB2[x] = random(956)-700;
        #         RGB2[x] = constrain(RGB2[x], 0, 255);
        #
        #         delay(1000);
        #        }
      end
    end
  end
end
