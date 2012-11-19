require 'settingslogic'
require 'fileutils'
#YAML::ENGINE.yamler= 'syck'

module NixieBerry
  class Settings < Settingslogic
    namespace ENV['NIXIE_BERRY_ENVIRONMENT']
    class << self
      def config_path
        file_name = "nixieberry-settings.yml"
        local_config_file = File.expand_path(File.join(File.dirname(__FILE__), "..", "..", "..", "config", file_name))
        home_config_file = File.join(Dir.home, file_name)
        config_path = local_config_file
        #todo doesn't work!!! because it's an instance an not yet set....
        if FileUtils.uptodate?(local_config_file, %w(home_config_file))
          FileUtils.cp(local_config_file, home_config_file) unless File.exists?(home_config_file)
        end
        home_config_file
      end
    end
    source config_path
  end
end
