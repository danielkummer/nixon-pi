root = ::File.dirname(__FILE__)
require ::File.join( root, 'web_server' )

run NixonPi::WebServer

#thin -R config.ru start
#http://stackoverflow.com/questions/5015471/using-sinatra-for-larger-projects-via-multiple-files/5030173#5030173