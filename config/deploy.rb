set :application, "nixon-pi"
set :repository, "git@github.com:danielkummer/nixon-pi.git"

set :scm_username, "daniel.kummer@gmail.com"

server 'nixonpi.namics.com', :app, :web, :db

set :deploy_to, "/home/pi/nixon-pi"
set :deploy_via, :copy
set :copy_strategy, :export


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

namespace :deploy do

  task :start do
    run "cd #{current_path} && bin/nixon-pi -e production"
  end

  task :stop do
    kill_processes_matching application
  end
  task :restart, :roles => :app, :except => {:no_release => true} do
    stop
    start
  end
end


def kill_processes_matching(name)
  run "ps -ef | grep #{name} | grep -v grep | awk '{print $2}' | xargs kill || echo 'no process with name #{name} found'"
end