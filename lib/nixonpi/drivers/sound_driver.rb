require_relative '../logging/logging'
require_relative '../information/os_info'
require_relative '../messaging/command_listener'
#require 'festivaltts4r'

#todo maybe remove string patch alltogether - doen't make much sense...
class String
  def to_speech(params={})
    text = params[:text] || self
    text = text.to_s.gsub(/_/, " ")
    if params[:flite]
      language = "-voice #{params[:lanuage]}" if params[:language] #voices: #kal awb_time kal16 awb rms slt
      language ||= ""
      cmd = "flite #{language}\"#{text}\""
    end
    self.class.execute cmd
  end

  def to_sound
    sound = self
    file = Dir.glob("**/*.mp3").select { |v| v=~ /#{sound}/ }.first
    unless file.nil?
      cmd = "/usr/local/bin/mpg123 \"#{File.expand_path(file)}\""
      self.class.execute cmd
    end
  end

  def self.execute(cmd)
    begin
      IO.popen(cmd)
    rescue StandardError => err
      log.error err.message
      raise err
    end
  end
end

module NixonPi
  class SoundDriver
    include Logging
    include OSInfo
    include CommandListener

    accepted_commands :value

    def handle_command(command)
      value = command[:value]
      log.info "got sound command: #{command}"

      case true
        when OSInfo.mac?
          IO.popen("say #{value}")
        when OSInfo.windows?
          log.warn "No windows speech support at the moment..."
        else
          if value.include?(".mp3")
            value.to_sound
          else
            value.to_speech
          end
      end
    end
  end
end