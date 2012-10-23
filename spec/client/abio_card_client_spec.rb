require_relative '../spec_helper'

describe AbioCardClient do

  before :each do
    connection = MockTelnet.new #create mock connection
    @client = AbioCardClient.instance
    @client.connection = connection

  end

end