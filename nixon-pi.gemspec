# -*- encoding: utf-8 -*-
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)

require 'nixon_pi/version'

Gem::Specification.new do |gem|
  gem.name          = 'nixon_pi'
  gem.version       = NixonPi::VERSION
  gem.authors       = ['Daniel Kummer']
  gem.email         = %w(daniel.kummer@gmail.com)
  gem.description   = 'Drive nixie tubes over a raspberry pi abiocard shield'
  gem.summary       = 'This service allows the control of ogilumen nixie tubes, nixie bar graphs and leds over encapsulating the telnet service'
  gem.homepage      = ''
  gem.required_ruby_version = '>=1.9'
  gem.has_rdoc      = true

  gem.files         = `git ls-files`.split($INPUT_RECORD_SEPARATOR)
  #gem.executables   = gem.files.grep(%r{^bin/}).map { |f| File.basename(f) }
  gem.executables   = ["nixon-pi"]
  gem.default_executable = 'nixon-pi'
  gem.test_files    = gem.files.grep(%r{^(test|spec|features)/})
  gem.require_paths = %w(lib config)

  gem.add_dependency 'activesupport', '~> 4.2.1'
  gem.add_dependency 'settingslogic', '~> 2.0.9'
  gem.add_dependency 'state_machine', '~> 1.2.0'
  # gem.add_dependency 'festivaltts4r'
  gem.add_dependency 'rufus-scheduler', '~> 3.1.1'
  gem.add_dependency 'foreman', '~> 0.78.0'
  gem.add_dependency 'bunny', '~> 0.9.8'#
  gem.add_dependency 'sinatra', '~> 1.4.6'
  gem.add_dependency 'sinatra-contrib', '~> 1.4.2'
  gem.add_dependency 'sinatra-activerecord', '~> 2.0.6'
  gem.add_dependency 'sequel', '~> 4.22.0' #todo do i need this?
  gem.add_dependency 'sqlite3', '~> 1.3.10'
  gem.add_dependency 'sinatra-formhelpers', '~> 0.4.0'
  gem.add_dependency 'sinatra-jsonp', '~> 0.4.4'
  gem.add_dependency 'json', '~> 1.8.2'
  gem.add_dependency 'thor', '~> 0.19.1'

  gem.add_dependency 'haml', '~> 4.0.6'
  gem.add_dependency 'chronic_duration', '~> 0.10.6'
  gem.add_dependency 'thin', '~> 1.6.3'
  gem.add_dependency 'colorize', '~> 0.7.7'
  #gem.add_dependency 'vegas'
  gem.add_dependency 'eventmachine', '~> 1.0.7'
  gem.add_dependency 'em_pessimistic', '~> 0.2.0'

  gem.add_development_dependency 'less', '~> 2.6.0'
  gem.add_development_dependency 'rdoc', '~> 4.2.0'
  gem.add_development_dependency 'rvm-capistrano', '~> 1.5.0'
  gem.add_development_dependency 'rspec', '~> 3.2.0'
  gem.add_development_dependency 'shoulda-matchers', '~> 2.8.0'
  gem.add_development_dependency 'bundler', '~> 1.9.4'
  gem.add_development_dependency 'simplecov', '~> 0.10.0'
  gem.add_development_dependency 'simplecov-gem-adapter', '~> 1.0.1'
  gem.add_development_dependency 'mocha','~> 1.1.0'
  gem.add_development_dependency 'ruby-graphviz', '~> 1.2.2'
  gem.add_development_dependency 'capistrano', '~> 3.0'
  gem.add_development_dependency 'capistrano-bundler', '~> 1.1'
  gem.add_development_dependency 'capistrano-rvm', '~> 0.1'
  gem.add_development_dependency 'capistrano-scm-gitcopy', '~> 0.0.8'
end
