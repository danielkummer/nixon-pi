namespace :app do
  desc 'Check if application is running'
  task :status do
    on roles(:app) do
      info execute "ps -ef | grep #{application} | grep -v grep || echo 'no process with name #{application} found'"
    end
  end
end
