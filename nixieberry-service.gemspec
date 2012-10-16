# -*- encoding: utf-8 -*-
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'nixieberry-service/version'

Gem::Specification.new do |gem|
  gem.name          = "nixieberry-service"
  gem.version       = Nixieberry::Service::VERSION
  gem.authors       = ["Daniel Kummer"]
  gem.email         = ["daniel.kummer@gamil.com"]
  gem.description   = %q{Drive nixie tubes over a raspberry pi abiocard shield}
  gem.summary       = %q{This service allows the control of ogilumen nixie tubes, nixie bar graphs and leds over encapsulating the telnet service}
  gem.homepage      = ""

  gem.files         = `git ls-files`.split($/)
  gem.executables   = gem.files.grep(%r{^bin/}).map{ |f| File.basename(f) }
  gem.test_files    = gem.files.grep(%r{^(test|spec|features)/})
  gem.require_paths = ["lib", "config"]

  gem.add_dependency 'activesupport', '>= 3.2.8'
  gem.add_dependency 'redis'
  gem.add_dependency 'daemons'
  gem.add_dependency 'state_machine'

  gem.add_development_dependency 'rdoc'
  gem.add_development_dependency 'rspec'
  gem.add_development_dependency 'bundler'
  gem.add_development_dependency 'simplecov'
  gem.add_development_dependency 'mocha'
  gem.add_development_dependency 'ruby-graphviz'


end

