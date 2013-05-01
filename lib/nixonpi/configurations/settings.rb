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
      # Get the path to the configuration file
      def config_path
        File.expand_path(File.join(File.dirname(__FILE__), "..", "..", "..", "config", "nixonpi-settings.yml"))
      end
    end

    source config_path
  end
end
