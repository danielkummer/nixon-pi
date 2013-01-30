set :application, "nixon-pi"
set :repository, "git@github.com:danielkummer/nixon-pi.git"

set :scm_username, "daniel.kummer@gmail.com"

server 'nixonpi', :app, :web, :db

set :deploy_to, "/home/pi/nixon-pi"
set :git_shallow_clone, 1

#set :deploy_via, :copy
#set :copy_strategy, :export

set :copy_local_tar, "/usr/bin/gnutar" if `uname` =~ /Darwin/


require "rvm/capistrano"                               # Load RVM's capistrano plugin.

set :rvm_ruby_string, '1.9.3@global'                     # Or:
#set :rvm_ruby_string, ENV['GEM_HOME'].gsub(/.*\//,"") # Read from local system
set :rvm_type, :system

after "deploy:restart", "deploy:cleanup"


set :user, "pi"
set :password, "pi"
set :use_sudo, false


namespace :app do
  desc "Check if appliction is running"
  task :status do
    run "ps -ef | grep #{application} | grep -v grep || echo 'no process with name #{application} found'"
  end
end

def kill_processes_matching(name)
  run "ps -ef | grep #{name} | grep -v grep | awk '{print $2}' | xargs kill || echo 'no process with name #{name} found'"
end

after 'deploy:update', 'bundle:install'
after 'deploy:update', 'foreman:export'
after 'deploy:update', 'foreman:restart'

namespace :bundle do
  desc "Installs the application dependencies"
  task :install, :roles => :app do
    run "cd #{current_path} && bundle --without development test"
  end
end

namespace :foreman do
  desc "Export the Procfile to Ubuntu's upstart scripts"
  task :export, :roles => :app do
    run "cd #{release_path} && rvmsudo bundle exec foreman export upstart /etc/init " +
            "-f ./Procfile.production -a #{application} -u #{user} -l #{shared_path}/log"
  end

  desc "Start the application services"
  task :start, :roles => :app do
    sudo "start #{application}"
  end

  desc "Stop the application services"
  task :stop, :roles => :app do
    sudo "stop #{application}"
  end

  desc "Restart the application services"
  task :restart, :roles => :app do
    run "sudo start #{application} || sudo restart #{application}"
  end

  desc "Display logs for a certain process - arg example: PROCESS=web-1"
  task :logs, :roles => :app do
    run "cd #{current_path}/log && cat #{ENV["PROCESS"]}.log"
  end
end

# stub out deploy:restart
namespace :deploy do
  task :restart do
  end
end