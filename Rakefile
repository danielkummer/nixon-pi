require 'bundler/gem_tasks'

require 'tasks/state_machine'
require 'sinatra/activerecord/rake'

require 'sinatra'
require 'sinatra/activerecord'

require 'lib/nixonpi/configurations/settings'

set :database, {adapter: "sqlite3", database: Settings.database}

Dir.glob('tasks/**/*.rake').each(&method(:import))