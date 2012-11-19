require 'festivaltts4r'

require_relative '../command_queue'
require_relative '../control_parameters'
require_relative 'handler_state_machine'


module NixieBerry
  class TalkHandler < HandlerStateMachine

    register_as :say

    #Todo kinda hack basing it on the handlerstatemachine superclass
    def write
      @current_state_parameters[:value].to_speech
    end
  end
end