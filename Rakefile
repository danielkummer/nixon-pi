require 'bundler/gem_tasks'
require 'sinatra'
require 'sinatra/activerecord'
require 'tasks/state_machine'


# Fix schema path
ENV['SCHEMA'] = 'lib/nixon_pi/db/schema.rb'
require 'sinatra/activerecord/rake'


# Constants for sinatra assetpack
APP_FILE  = 'lib/nixon_pi/web/web_server.rb'
APP_CLASS = 'NixonPi::WebServer'
require 'sinatra/assetpack/rake'
# This makes the following services available:
#rake assetpack:precompile           # Precompile all assets
#rake assetpack:precompile:files     # Precompile files only
#rake assetpack:precompile:packages  # Precompile packages only


# Fix migrations path
ActiveRecord::Migrator.migrations_paths = 'lib/nixon_pi/db/migrate'

# Hack so you're able to supply the environment via bash environment variable
$environment = ENV['ENV'] || 'development'
require 'nixon_pi/settings'


set :database, {adapter: "sqlite3", database: NixonPi::Settings.full_database_path}

Dir.glob('tasks/**/*.rake').each(&method(:import))