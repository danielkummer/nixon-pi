require 'festivaltts4r'

require_relative '../command_queue'
require_relative '../command_parameters'
require_relative 'handler_state_machine'
require_relative '../../os'
require_relative '../logging/logging'


module NixieBerry
  class TalkHandler < HandlerStateMachine
    include OS
    include Logging

    register_as :say

    def after_create
      CommandProcessor.add_receiver(self, :say)
    end

    def write
      log.debug "Say: #{current_state_parameters[:value].to_speech}"
      case true
        when OS.mac?
          IO.popen("say #{current_state_parameters[:value]}")
        when OS.windows?
          log.warn "No windows speech support at the moment..."
        else
          current_state_parameters[:value].to_speech
      end
    end
  end
end