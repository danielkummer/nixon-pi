require_relative 'spec_helper'
require 'moqueue'

require_relative '../lib/nixonpi/messaging/messaging'


describe CommandSender do
  before :all do
    overload_amqp
  end



  it "should send a command to the registered receivers" do
    cs = CommandSender.new
    cmd = {test: 'command'}
    cs.send_command(:tubes, cmd)


  end




  #queue.should have_received("a message")
  #queue.should have_ack_for("a different message")

end

