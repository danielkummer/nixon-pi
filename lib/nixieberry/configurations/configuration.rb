require 'yaml'
require 'active_support/core_ext/hash/indifferent_access'

module NixieBerry
  module Configuration
    def config
      @@config ||= Configuration.config
    end

    class << self
      def config
        config_path = File.expand_path(File.join(File.dirname(__FILE__), "..", "..", "..", "config", "config.yml"))
        config_path = ARGV.first if ARGV.first and File.exists?(ARGV[0]) and ENV['NIXIE_BERRY_ENVIRONMENT'].to_sym == :production

        config_hash = HashWithIndifferentAccess.new(YAML.load_file(config_path))[ENV['NIXIE_BERRY_ENVIRONMENT']]
        config_hash
      end
    end
  end
end
