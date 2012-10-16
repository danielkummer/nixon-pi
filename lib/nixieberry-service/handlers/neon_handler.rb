module NixieBerry
  class NeonHandler
    def initialize
      @driver = NixieBerry::NeonDriver.instance
      @controlconfig = NixieBerry::ControlConfiguration.instance
    end

    def write_neon
      action = @controlconfig[:neon]
      case action
        when "on"
          @d
        when "off"
        when "blink"
          #
        else
          log.error "unspecified neon action: #{action}"
      end
    end
  end
end