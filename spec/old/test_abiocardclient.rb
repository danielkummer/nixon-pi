require 'helper'

class TestAbiocardClient < Test::Unit::TestCase

  def setup
    @client = nil
  end

  should "write a pin" do
    assert @client.write_io_pin(1, 0)
    assert @client.write_io_pin(1, 1)
    assert @client.write_io_pin(1, 0)
    assert @client.write_io_pin(1, 1)
  end

  should "read a pin" do
    assert_equal 0, @client.read_io_pin(1)
 end

end
