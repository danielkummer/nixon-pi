# config valid only for current version of Capistrano
lock '3.4.0'

set :application, 'nixon-pi'
set :scm, :gitcopy
set :repo_url, 'git@github.com:danielkummer/nixon-pi.git'

set :stage, :production

# We will tell a white lie to Capistrano

server '10.0.1.14', user: 'pi', roles: %w{app db web}, my_property: :my_value

set :deploy_to, '/home/pi/nixon-pi'

set :branch, Regexp.last_match(1) if `git branch` =~ /\* (\S+)\s/m

set :user, 'pi'
set :password, 'pi'


after 'deploy:restart', 'deploy:cleanup'
after 'deploy:update', 'deploy:link_db'
after 'deploy:update', 'foreman:export'
after 'deploy:update', 'deploy:link_init'


#USAGE:
#cap uat deploy -s branch=(your release branch)


# Default branch is :master
# ask :branch, `git rev-parse --abbrev-ref HEAD`.chomp

# Default deploy_to directory is /var/www/my_app_name
# set :deploy_to, '/var/www/my_app_name'

# Default value for :scm is :git
# set :scm, :git

# Default value for :format is :pretty
# set :format, :pretty

# Default value for :log_level is :debug
# set :log_level, :debug

# Default value for :pty is false
# set :pty, true

# Default value for :linked_files is []
# set :linked_files, fetch(:linked_files, []).push('config/database.yml', 'config/secrets.yml')

# Default value for linked_dirs is []
# set :linked_dirs, fetch(:linked_dirs, []).push('log', 'tmp/pids', 'tmp/cache', 'tmp/sockets', 'vendor/bundle', 'public/system')

# Default value for default_env is {}
# set :default_env, { path: "/opt/ruby/bin:$PATH" }

# Default value for keep_releases is 5
# set :keep_releases, 5

