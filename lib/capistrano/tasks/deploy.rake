namespace :deploy do
  desc 'Link production db into current directory'
  task :link_db do
    on roles(:db) do
      execute "rm -f  #{latest_release}/db/settings.db"
      execute "ln -s #{shared_path}/db/settings.db #{latest_release}/db/settings.db"
    end
  end

  desc 'Link the init script into init.d'
  task :link_init do
    on roles(:web) do
      execute "sudo rm -f /etc/init.d/#{application}"
      execute "sudo ln -s #{latest_release}/config/bluepill-init.sh /etc/init.d/#{application}"
      execute "sudo chmod 777 /etc/init.d/#{application}"
      execute 'sudo chmod 777 /var/run' # to create bluepill directory on start
    end
  end

  task :restart do
  end
end