require_relative '../../lib/nixieberry/logging/logging'


class MockTelnet
  include NixieBerry::Logging

  def cmd(string)
    #enable for more debug output..
    #STDERR.puts "telnet mock: " << string.to_s
    $last_cmd = string


    case string
      when "ER"
        ret = "ERFF"
      when "CR"
        ret = "CR" << ("1" * 12) << "01" #last two are powerup and battery
      when "HI"
        ret = "HI0F"
      when /PR.*/
        ret = "PR0011FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" #all pwm on 255
      else
        ret = ""
    end

    if string.class == Hash
      if string['String'] == 'HI'
        ret = "HIFF"
      end
    end

    if block_given?
      yield ret
    end
  end
end