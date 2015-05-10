require_relative '../../logging/logging'
require_relative '../../information/os_info'
require_relative '../../messaging/commands_module'
require_relative '../../information/information_holder'

module NixonPi
  class SoundProxy
    include Logging
    extend Logging
    include OSInfo
    include Commands
    include InformationHolder

    accepted_commands :value

    def handle_command(command)
      value = command[:value]
      log.debug "got sound command: #{command}"

      case true
        when OSInfo.mac?
          IO.popen("say #{value}")
        when OSInfo.windows?
          log.warn 'No windows speech support at the moment...'
        else
          value.include?('.mp3') ? sound(value) : speech(value)
      end
    end

    def handle_info_request(about)
      ret = {}
      case about.to_sym
        when :commands
          ret = { commands: self.class.available_commands }
        else
          log.error "No information about #{about}"
      end
      ret
    end

    private

    def speech(text, params = {})
      text = text.to_s.gsub(/_/, ' ')
      language = "-voice #{params[:language]}" if params[:language] # voices: #kal awb_time kal16 awb rms slt
      language ||= '-voice rms'
      cmd = "flite #{language} -t \"#{text}\""
      self.class.execute cmd
    end

    def sound(filename)
      sound = filename
      file = Dir.glob('**/*.mp3').find { |v| v =~ /#{sound}/ }
      unless file.nil?
        cmd = "/usr/local/bin/mpg123 \"#{File.expand_path(file)}\""
        self.class.execute cmd
      end
    end

    def self.execute(cmd)
      pid = fork do
        IO.popen(cmd)
      end
      Process.detach(pid)
    rescue StandardError => err
      log.error err.message
      raise err
    end
  end
end
