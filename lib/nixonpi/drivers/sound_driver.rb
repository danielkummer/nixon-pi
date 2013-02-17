require_relative '../logging/logging'
require_relative '../information/os_info'
require_relative '../messaging/command_listener'


module NixonPi
  class SoundDriver
    include Logging
    extend Logging
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
          value.include?(".mp3") ? sound(value) : speech(value)
      end
    end

    def speech(text, params={})
      text = text.to_s.gsub(/_/, " ")
      language = "-voice #{params[:language]}" if params[:language] #voices: #kal awb_time kal16 awb rms slt
      language ||= ""
      cmd = "flite -t #{language}\"#{text}\""
      self.class.execute cmd
    end

    def sound(filename)
      sound = filename
      file = Dir.glob("**/*.mp3").select { |v| v=~ /#{sound}/ }.first
      unless file.nil?
        cmd = "/usr/local/bin/mpg123 \"#{File.expand_path(file)}\""
        self.class.execute cmd
      end
    end

    def self.execute(cmd)
      @last_time ||= Time.now
      now = Time.now

      #todo not good enough - silent drop of messages
      if now >= @last_time + 5.seconds
        begin
          IO.popen(cmd)
        rescue StandardError => err
          log.error err.message
          raise err
        end
      end unless @last_time.nil?
      @last_time = now
    end
  end
end