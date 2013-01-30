require_relative '../logging/logging'
require_relative '../information/os_info'
require_relative '../messaging/command_listener'

module NixonPi
  class SpeechDriver
    include Logging
    include OSInfo
    include CommandListener

    accepted_commands :value

    def handle_command(command)
      value = command[:value]
      log.info "got speech command: say #{command}"
      case true
        when OSInfo.mac?
          IO.popen("say #{value}")
        when OSInfo.windows?
          log.warn "No windows speech support at the moment..."
        else
          value.to_speech
      end
    end

  end
end