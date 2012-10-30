require_relative '../logging/logging'
require_relative '../configurations/settings'

#todo refactor for usage with pwm and any number of lamps
module NixieBerry
  class LampDriver
    include Logging

    def initialize
      @client = NixieBerry::AbioCardClient.instance
      @controlconfig = NixieBerry::Control.instance
      @pin_array = Settings.in1_pins
    end

    def on(lamp_number)
      @client.pwm_write(@pin_array[lamp_number], 255)
    end

    def off(lamp_number)
      @client.pwm_write(@pin_array[lamp_number], 0)
    end

=begin
    #todo merge with generic animation module
    def stop_blinking
      Thread.kill(@blink_thread)
    end

    def start_blinking(frequency_in_seconds = 1, times = nil)
      @blink_thread = Thread.new do
        if times
          times.times { self.blink(frequency_in_seconds) }
        else
          loop { self.blink(frequency_in_seconds) }
        end
      end
    end
  end

  private
  def blink(frequency_in_seconds)
    self.on
    sleep frequency_in_seconds
    self.off
    sleep frequency_in_seconds
  end
=end

end
