module NixieBerry
  class NeonDriver
    include NixieLogger

    def initialize
      @client = NixieBerry::AbioCardClient.instance
      @controlconfig = NixieBerry::ControlConfiguration.instance
      @pin = config[:neon_pin]
    end

    def on
      @client.io_write(@pin, 1)
    end

    def off
      @client.io_write(@pin, 0)
    end

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

end
