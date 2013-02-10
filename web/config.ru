require File.join( File.dirname(__FILE__), 'web_server' )

disable :run

map '/' do
  run NixonPi::WebServer
end

