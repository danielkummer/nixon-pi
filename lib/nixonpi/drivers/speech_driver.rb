require_relative '../logging/logging'
require_relative '../information/os_info'
require_relative '../messaging/command_listener'
require 'festivaltts4r'

class String
  def to_speech(params={})
    text = params[:text] || self
    text = text.to_s.gsub(/_/, " ")
    if params[:flite]
      #kal awb_time kal16 awb rms slt
      language = "-voice #{params[:lanuage]}" if params[:language]
      language ||= ""
      cmd = "flite #{language}\"#{text}\""
    else
      festival = params[:festival] || "festival --tts"
      language = "--language " + params[:language] if params[:language]
      cmd = "echo \"#{text}\" | #{festival} #{language} 2>&1"

    end


    self.class.execute cmd
  end

end

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
          value.to_speech({flite: true})
      end
    end

  end
end