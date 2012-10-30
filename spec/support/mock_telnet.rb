require_relative '../../lib/nixieberry/logging/logging'


class MockTelnet
  include NixieBerry::Logging

  def cmd(string)
    #STDERR.puts(string + " binary: " + string[2..3].to_i(16).to_s(2).rjust(8, '0'))
    STDERR.puts "telnet mock: " << string
    $last_cmd = string

    case string
      when "ER"
        ret = "ERFF"
      when "CR"
        ret = "CR" << ("1" * 12) << "01" #last two are powerup and battery
      when "HI"
        ret = "HI0F"
      when /PR.*/
        ret = "PR0011FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" #all pwm on 255
      else
        ret = ""
    end

    if block_given?
      yield ret
    end
  end
end