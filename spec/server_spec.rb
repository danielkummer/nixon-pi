require_relative 'spec_helper'
require_relative '../lib/nixieberry/web/web_server'


describe NixieBerry::WebServer, exclude: true do

  it "should start a new webserver" do
    @server = NixieBerry::WebServer.run!

    @server
  end



end