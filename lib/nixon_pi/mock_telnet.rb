module NixonPi
  class MockTelnet
    include Logging

    def close
      log.debug 'Closing connection'
    end

    def cmd(string)
      # enable for more debug output..
      # STDERR.puts "telnet mock: " << string.to_s
      $last_cmd = string

      case string
        when 'ER'
          ret = 'ERFF'
        when 'CR'
          ret = 'CR' << ('1' * 12) << '01' # last two are powerup and battery
        when 'HI'
          ret = 'HI0F'
        when /PR.*/
          ret = 'PR0011FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF' # all pwm on 255
        else
          ret = ''
      end

      ret = 'HIFF' if string['String'] == 'HI' if string.class == Hash

      yield ret if block_given?
    end
  end
end
