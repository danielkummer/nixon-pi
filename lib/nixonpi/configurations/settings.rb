require 'settingslogic'
require 'fileutils'

YAML::ENGINE.yamler= 'syck'

##
# Monkeypath settingslogic
class Settingslogic

  ##
  # fixes rspec to_ary error according to https://github.com/binarylogic/settingslogic/commit/d623622f7d8b184aebe9fda6c7996c4a44af5ee9
  def method_missing(name, *args, &block)
    super if name === :to_ary # delegate to_ary to Hash
    key = name.to_s
    return missing_key("Missing setting '#{key}' in #@section") unless has_key? key
    value = fetch(key)
    create_accessor_for(key)
    value.is_a?(Hash) ? self.class.new(value, "'#{key}' section in #@section") : value
  end

end

module NixonPi
  class Settings < Settingslogic
    namespace ENV['RACK_ENV']

    class << self

      ##
      # Get the path to the configuration file, create the file in the users home directory if it doesn't exist or is outdated
      def path_to_config
        gem_path = File.expand_path(File.join(File.dirname(__FILE__), "..", "..", "..", "config", "nixonpi-settings.yml"))
        home_path = File.join(Dir.home, "nixonpi-settings.yml")

        if ENV['RACK_ENV'] == 'development' or FileUtils.uptodate?(gem_path, %w(home_path))
          FileUtils.cp(gem_path, home_path) unless File.exists?(home_path)
        end

        home_path
      end

      ##
      # Save the configuration as yaml file back
      def save
        complete_config = YAML.load(ERB.new(File.read(path_to_config)).result).to_hash
        complete_config[@namespace].merge!(instance)
        File.open(path_to_config, 'w+') { |f| f.write(complete_config.to_yaml) }
      end

    end

    source path_to_config
  end
end
