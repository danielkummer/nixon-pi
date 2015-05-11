
ENV['RACK_ENV'] ||= 'test'

require 'rubygems'
require 'bundler/setup'
require 'simplecov'
require 'simplecov-gem-adapter'
require_relative 'support/active_record'

SimpleCov.start 'gem'

$environment  = 'test'

RSpec.configure do |config|
  config.run_all_when_everything_filtered = true
  # config.filter_run :focus
  config.filter_run_excluding exclude: true
end
