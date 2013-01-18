require 'singleton'
require_relative '../logging/logging'
require_relative '../../os'

module NixonPi
  class SpeechDriver
    include Logging
    include OS
    include Singleton

    def receive(command)
      value = command[:value]
      log.debug "got speech command: say #{command}"
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