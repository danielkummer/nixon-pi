require_relative '../../lib/logging/nixie_logger'

class MockTelnet
  include NixieLogger

  def cmd(string)
    log.debug(string + " binary: " + string[2..3].to_i(16).to_s(2).rjust(8,'0'))

    case string
      when "ER"
        ret = "ERFF"
      else
        ret = ""
    end
    return ret
  end
end