# -*- encoding: utf-8 -*-
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'nixieberry/version'

Gem::Specification.new do |gem|
  gem.name          = "nixieberry"
  gem.version       = NixieBerry::Service::VERSION
  gem.authors       = ["Daniel Kummer"]
  gem.email         = ["daniel.kummer@gmail.com"]
  gem.description   = %q{Drive nixie tubes over a raspberry pi abiocard shield}
  gem.summary       = %q{This service allows the control of ogilumen nixie tubes, nixie bar graphs and leds over encapsulating the telnet service}
  gem.homepage      = ""
  gem.required_ruby_version = '>=1.9'
  gem.has_rdoc      = true

  gem.files         = `git ls-files`.split($/)
  gem.executables   = gem.files.grep(%r{^bin/}).map{ |f| File.basename(f) }
  gem.default_executable = 'nixieberry-daemon'
  gem.test_files    = gem.files.grep(%r{^(test|spec|features)/})
  gem.require_paths = ["lib", "config"]

  gem.add_dependency 'activesupport', '>= 3.2.8'
  gem.add_dependency 'settingslogic'
  gem.add_dependency 'daemons'
  gem.add_dependency 'state_machine'
  gem.add_dependency 'festivaltts4r'
  gem.add_dependency 'webrick'
  gem.add_dependency 'sinatra'
  gem.add_dependency 'sinatra-contrib'
  gem.add_dependency 'json'
  gem.add_dependency 'haml'

  gem.add_development_dependency 'rdoc'
  gem.add_development_dependency 'rspec'
  gem.add_development_dependency 'bundler'
  gem.add_development_dependency 'simplecov'
  gem.add_development_dependency 'simplecov-gem-adapter'
  gem.add_development_dependency 'mocha'
  gem.add_development_dependency 'ruby-graphviz'


end

