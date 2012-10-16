require 'mock_abiocardclient'

class TestAbiocardClient < Test::Unit::TestCase

  def setup
    @client = MockAbiocardClient.new
    @nixie = NixieBerry::TubeDriver.new

  end

  should "write an array" do
    @nixie.write([1,2,3,4,5,6])
  end

end
