#!/usr/bin/env ruby
#require 'profile'

$environment  = 'development' #default value if nothing else is set

if ARGV.any?
  require 'optparse'
  OptionParser.new { |op|
    op.on('-e env', 'set the environment (default is development, others are test and production)') do |val|
      $environment  = val
    end
    op.on('-p port', 'set the webserver port (default is 8080)') do |val|
      require_relative '../lib/nixieberry/configurations/settings'
      NixieBerry::Settings['web_server']['port'] = val
    end
    op.on('-h', '--help', 'Display help') do
      puts op
      exit
    end
  }.parse!(ARGV.dup)
end
require_relative '../lib/nixieberry_service'
server = NixieBerry::NixieService.new
server.run

