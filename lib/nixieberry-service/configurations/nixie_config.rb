require 'yaml'
require 'active_support/core_ext/hash/indifferent_access'

module NixieConfig
  def config
    @@config ||= NixieConfig.config
  end

  class << self
    def config
      config_path = File.expand_path(File.join(File.dirname(__FILE__), ".."  ,"..", "..", "config", "config.yml"))
      config_path = ARGV.first if ARGV.first and File.exists?(ARGV[0])
      HashWithIndifferentAccess.new(YAML.load_file(config_path))[CONFIG_ENV]
    end
  end

end
