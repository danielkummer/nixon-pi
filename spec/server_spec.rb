require_relative 'spec_helper'
require_relative '../lib/nixieberry/web/web_server'


describe NixonPi::WebServer, exclude: true do

  it "should start a new webserver" do
    @server = NixonPi::WebServer.run!

    @server
  end



end