require 'rubygems'
require 'bundler/setup'
require 'simplecov'
require 'simplecov-gem-adapter'
SimpleCov.start 'gem'


ENV['NIXIE_BERRY_ENVIRONMENT'] = 'test'

RSpec.configure do |config|
  config.treat_symbols_as_metadata_keys_with_true_values = true
  config.run_all_when_everything_filtered = true
  config.filter_run :focus
  config.filter_run_excluding :exclude => true
end
