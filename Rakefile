require 'bundler/gem_tasks'

require 'tasks/state_machine'
require 'sinatra/activerecord/rake'

require 'sinatra'
require 'sinatra/activerecord'

require 'nixon_pi/settings'

#for sinatra assetpack
APP_FILE  = 'lib/web/web_server.rb'
APP_CLASS = 'WebServer'

require 'sinatra/assetpack/rake'

#rake assetpack:precompile           # Precompile all assets
#rake assetpack:precompile:files     # Precompile files only
#rake assetpack:precompile:packages  # Precompile packages only

set :database, {adapter: "sqlite3", database: NixonPi::Settings.database}

Dir.glob('tasks/**/*.rake').each(&method(:import))