require_relative '../lib/nixieberry_service'
require 'profile'

server = NixieBerry::NixieService.new
server.run

