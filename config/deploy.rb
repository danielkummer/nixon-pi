set :application, 'nixon-pi'
set :scm, :none
set :repository, '.'

server 'nixonpi', :app, :web, :db

set :deploy_to, '/home/pi/nixon-pi'
set :deploy_via, :copy
set :branch, Regexp.last_match(1) if `git branch` =~ /\* (\S+)\s/m

set :copy_local_tar, '/usr/bin/gnutar' if `uname` =~ /Darwin/

require 'rvm/capistrano' # Load RVM's capistrano plugin.

set :rvm_ruby_string, '1.9.3@global' # Or: :rvm_ruby_string, ENV['GEM_HOME'].gsub(/.*\//,"") # Read from local system
set :rvm_type, :system
set :rvm_path, '/usr/local/rvm'

after 'deploy:restart', 'deploy:cleanup'

set :user, 'pi'
set :password, 'pi'
set :use_sudo, false

namespace :app do
  desc 'Check if appliction is running'
  task :status do
    run "ps -ef | grep #{application} | grep -v grep || echo 'no process with name #{application} found'"
  end
end

def kill_processes_matching(name)
  run "ps -ef | grep #{name} | grep -v grep | awk '{print $2}' | xargs kill || echo 'no process with name #{name} found'"
end

after 'deploy:update', 'bundle:install'
after 'deploy:update', 'deploy:link_db'
after 'deploy:update', 'foreman:export'
after 'deploy:update', 'deploy:link_init'

namespace :bundle do
  desc 'Installs the application dependencies'
  task :install, roles: :app do
    run "cd #{current_path} && bundle --without development test"
  end
end

namespace :foreman do
  desc "Export the Procfile to Ubuntu's upstart scripts"
  task :export, roles: :app do
    # run "cd #{release_path} && rvmsudo bundle exec foreman export bluepill ./config " +
    #        "-f ./Procfile.production -a #{application} -u #{user} -l #{shared_path}/log"
    run "cd #{release_path} && rvmsudo bundle exec foreman export bluepill ./config " \
            "-f ./Procfile.production -a #{application} -u root -l #{shared_path}/log"
  end
end

# stub out deploy:restart
namespace :deploy do
  desc 'Link production db into current directory'
  task :link_db do
    run "rm -f  #{latest_release}/db/settings.db"
    run "ln -s #{shared_path}/db/settings.db #{latest_release}/db/settings.db"
  end

  desc 'Link the init script into init.d'
  task :link_init do
    run "sudo rm -f /etc/init.d/#{application}"
    run "sudo ln -s #{latest_release}/config/bluepill-init.sh /etc/init.d/#{application}"
    run "sudo chmod 777 /etc/init.d/#{application}"
    run 'sudo chmod 777 /var/run' # to create bluepill directory on start
  end

  task :restart do
  end
end
