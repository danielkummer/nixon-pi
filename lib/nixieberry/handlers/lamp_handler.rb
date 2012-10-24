require_relative '../logging/logging'
require_relative '../drivers/lamp_driver'
require_relative '../configurations/control'

module NixieBerry
  class LampHandler
    def initialize
      @driver = NixieBerry::LampDriver.instance
      @controlconfig = NixieBerry::Control.instance
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