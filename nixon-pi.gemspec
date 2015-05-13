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

  gem.add_dependency 'activesupport', '>= 3.2.8'
  gem.add_dependency 'settingslogic'
  gem.add_dependency 'state_machine'
  # gem.add_dependency 'festivaltts4r'
  gem.add_dependency 'rufus-scheduler'
  gem.add_dependency 'foreman'
  gem.add_dependency 'bunny', '~> 0.9.0.pre4'
  gem.add_dependency 'sinatra'
  gem.add_dependency 'sinatra-contrib'
  gem.add_dependency 'sinatra-activerecord'
  gem.add_dependency 'sequel'
  gem.add_dependency 'sqlite3'
  gem.add_dependency 'sinatra-formhelpers'
  gem.add_dependency 'sinatra-jsonp'
  gem.add_dependency 'json'
  gem.add_dependency 'thor'

  gem.add_dependency 'haml'
  gem.add_dependency 'chronic_duration'
  gem.add_dependency 'thin'
  gem.add_dependency 'colorize'

  gem.add_development_dependency 'less'
  gem.add_development_dependency 'rdoc'
  gem.add_development_dependency 'rvm-capistrano'
  gem.add_development_dependency 'rspec'
  gem.add_development_dependency 'shoulda-matchers'
  gem.add_development_dependency 'bundler'
  gem.add_development_dependency 'simplecov'
  gem.add_development_dependency 'simplecov-gem-adapter'
  gem.add_development_dependency 'mocha'
  gem.add_development_dependency 'ruby-graphviz'
  gem.add_development_dependency 'capistrano', '~> 3.0'
  gem.add_development_dependency 'capistrano-bundler', '~> 1.1'
  gem.add_development_dependency 'capistrano-rvm', '~> 0.1'
  gem.add_development_dependency 'capistrano-scm-gitcopy'
end
