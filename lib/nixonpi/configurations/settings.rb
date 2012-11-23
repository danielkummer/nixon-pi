require 'settingslogic'
require 'fileutils'
YAML::ENGINE.yamler= 'syck'

module NixonPi
  class Settings < Settingslogic
    namespace $environment
    class << self
      def config_path
        file_name = "nixonpi-settings.yml"
        local_config_file = File.expand_path(File.join(File.dirname(__FILE__), "..", "..", "..", "config", file_name))
        home_config_file = File.join(Dir.home, file_name)
        if FileUtils.uptodate?(local_config_file, %w(home_config_file)) or  $environment  == 'development'
          FileUtils.cp(local_config_file, home_config_file) unless File.exists?(home_config_file)
        end
        home_config_file
      end
    end
    source config_path
  end
end
