require 'bundler/gem_tasks'

require 'tasks/state_machine'
require 'sinatra/activerecord/rake'

require 'sinatra'
require 'sinatra/activerecord'

require 'nixon_pi/settings'

set :database, {adapter: "sqlite3", database: NixonPi::Settings.database}

Dir.glob('tasks/**/*.rake').each(&method(:import))