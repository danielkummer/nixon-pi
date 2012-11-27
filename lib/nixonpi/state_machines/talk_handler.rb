require 'festivaltts4r'

require_relative '../command_queue'
require_relative '../command_parameters'
require_relative 'handler_state_machine'


module NixieBerry
  class TalkHandler < HandlerStateMachine

    register_as :say

    def write
      current_state_parameters[:value].to_speech
    end
  end
end