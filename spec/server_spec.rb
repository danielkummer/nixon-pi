require_relative 'spec_helper'
require_relative '../lib/nixieberry/web/sinatra_server'

describe NixieBerry::RESTServer do

  it "should start a new webserver" do
    @server = NixieBerry::RESTServer.run!

    @server
  end



end