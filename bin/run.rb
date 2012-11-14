require_relative '../lib/nixieberry_service'

server = NixieBerry::NixieService.new
server.run

