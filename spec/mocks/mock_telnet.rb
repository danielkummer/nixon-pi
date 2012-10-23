require_relative '../../lib/nixieberry-service/logging/nixie_logger'

module NixieBerry
  class MockTelnet
    include NixieLogger

    def cmd(string)
      log.debug(string + " binary: " + string[2..3].to_i(16).to_s(2).rjust(8, '0'))

      case string
        when "ER"
          ret = "ERFF"
        else
          ret = ""
      end
      return ret
    end
  end
end