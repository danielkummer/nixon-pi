require_relative 'spec_helper'
require_relative '../web/web_server'
require_relative 'support/active_record'
require 'sinatra'
require 'rack/test'

$environment = 'test'
set :environment, :test
# set :database, 'sqlite:///spec/db/settings.db'

describe 'The Nixon-Pi Web Application', exclude: true do
  include Rack::Test::Methods

  def app
    NixonPi::WebServer
  end

  it 'has an index page' do
    get '/'
    last_response.should be_ok
    last_response.body.should include 'github'
  end

  # todo
  # get post delete actions
end
