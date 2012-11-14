require 'settingslogic'
#YAML::ENGINE.yamler= 'syck'

module NixieBerry
  class Settings < Settingslogic
    source File.expand_path(File.join(File.dirname(__FILE__), "..", "..", "..", "config", "config.yml"))
    namespace ENV['NIXIE_BERRY_ENVIRONMENT'] || 'development'


    class << self
      #def yaml_file
      #  path = File.expand_path(File.join(File.dirname(__FILE__), "..", "..", "..", "config", "config.yml"))
      #  if ENV['NIXIE_BERRY_ENVIRONMENT'].to_sym == :production
      #    other_path = ARGV.first
      #    path = other_path if other_path and File.exists?(other_path)
      #  end
      #  path
      #end

    end
  end
end