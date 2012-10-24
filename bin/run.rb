require_relative '../lib/nixieberry_service'

server = NixieBerry::Service.new
server.run

