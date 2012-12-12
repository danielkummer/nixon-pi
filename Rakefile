require "bundler/gem_tasks"

require 'rspec/core/rake_task'
require 'tasks/state_machine'

require 'sinatra/activerecord/rake'

require 'sinatra'
require 'sinatra/activerecord'

set :database, 'sqlite3:///db/settings.db'

RSpec::Core::RakeTask.new('spec')
task :default => :spec
