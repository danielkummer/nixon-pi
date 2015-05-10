require 'bundler/gem_tasks'

# if $environment == 'development'
require 'rspec/core/rake_task'
require 'tasks/state_machine'
RSpec::Core::RakeTask.new('spec')
task default: :spec
# end

require 'sinatra/activerecord/rake'

require 'sinatra'
require 'sinatra/activerecord'

set :database, 'sqlite3:///db/settings.db'
