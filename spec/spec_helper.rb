
ENV['RACK_ENV'] ||= 'test'

require 'rubygems'
require 'bundler/setup'
require 'simplecov'
require 'simplecov-gem-adapter'
require 'shoulda/matchers'
require 'shoulda/matchers/integrations/rspec'


SimpleCov.start 'gem'


$environment  = 'test'

RSpec.configure do |config|
  config.treat_symbols_as_metadata_keys_with_true_values = true
  config.run_all_when_everything_filtered = true
  #config.filter_run :focus
  config.filter_run_excluding :exclude => true
end
