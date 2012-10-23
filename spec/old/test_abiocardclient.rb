require 'helper'

class TestAbiocardClient < Test::Unit::TestCase

  def setup
    @client = nil
  end

  should "write a pin" do
    assert @client.io_write(1, 0)
    assert @client.io_write(1, 1)
    assert @client.io_write(1, 0)
    assert @client.io_write(1, 1)
  end

  should "read a pin" do
    assert_equal 0, @client.io_read(0)
 end

end
