require 'logger'
require 'singleton'

class MockAbiocardClient
  include Singleton

  def initialize
    @log = Logger.new('../log/mock_client.log')
    @log.level = Logger::DEBUG
  end

  def write_pin(pin, value)
    @log.debug("write pin: #{pin}, Value: #{value}")
  end

  def read_pin(pin)
    @log.debug("read pin: #{pin}")
  end
end