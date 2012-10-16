require 'yaml'
require 'active_support/core_ext/hash/indifferent_access'

module NixieConfig
  def config
    @@config ||= HashWithIndifferentAccess.new(YAML.load_file(File.expand_path(File.join(File.dirname(__FILE__), ".."  ,"..", "config", "config.yml"))))[CONFIG_ENV]
  end
end