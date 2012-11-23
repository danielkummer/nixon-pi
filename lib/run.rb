#require 'profile'

ENV['NIXIE_BERRY_ENVIRONMENT'] = 'development' #default value if nothing else is set
ENV['DRIVER'] = 'telnet'

if ARGV.any?
  require 'optparse'
  OptionParser.new { |op|
    op.on('-e env', 'set the environment (default is development, others are test and production)') do |val|
      ENV['NIXIE_BERRY_ENVIRONMENT'] = val
    end
    op.on('-p port', 'set the webserver port (default is 8080)') do |val|
      require_relative '../lib/nixieberry/configurations/settings'
      NixieBerry::Settings['web_server']['port'] = val
    end
    op.on('-d', 'use direct io instead of telnet connection') do
      ENV['DRIVER'] = 'open3'
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

