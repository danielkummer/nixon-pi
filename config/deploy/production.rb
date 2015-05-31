set :stage, :production
server '10.0.1.8', user: 'pi', roles: %w{app db web}, my_property: :my_value

set :deploy_to, '/home/pi/nixon-pi'
set :user, 'pi'
set :password, 'pi'
