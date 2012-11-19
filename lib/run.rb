#require 'profile'

if ARGV.any?
  require 'optparse'
  OptionParser.new { |op|
    op.on('-e env', 'set the environment (default is development, others are test and production)') do |val|
      ENV['NIXIE_BERRY_ENVIRONMENT'] = val
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

