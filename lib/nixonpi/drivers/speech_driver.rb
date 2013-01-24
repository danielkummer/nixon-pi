require_relative '../logging/logging'
require_relative '../../os'
require_relative '../messaging/message_listener'

module NixonPi
  class SpeechDriver
    include Logging
    include OS
    include MessageListener

    accepted_commands :value

    def receive(command)
      value = command[:value]
      log.info "got speech command: say #{command}"
      case true
        when OS.mac?
          IO.popen("say #{value}")
        when OS.windows?
          log.warn "No windows speech support at the moment..."
        else
          value.to_speech
      end
    end

  end
end