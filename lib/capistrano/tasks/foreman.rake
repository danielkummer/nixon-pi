namespace :foreman do
  desc "Export the Procfile to Ubuntu's upstart scripts"

  task :export do
    on roles(:app) do
      within "#{release_path}" do
        execute :bundle, :exec, 'foreman export bluepill ./config ' \
                "-f ./Procfile.production -a #{fetch(:application)} -u root -l #{shared_path}/log"
      end
    end
  end

end
