require_relative 'spec_helper'

require_relative '../lib/nixonpi/rabbit_mq/command_receiver'

# describe NixonPi::RabbitMQ::CommandInbox do
#
#   before :each do
#     @cr = NixonPi::RabbitMQ::CommandInbox.new
#   end
#
#   it "should allow the registration of receivers who implement commandlistener" do
#     receiver = Object.new
#     receiver.extend(CommandListener)
#     expect { @cr.add_receiver(receiver, :command) }.not_to raise_error
#   end
#
#   it "should reject objects who don't implement CommandListener" do
#     receiver = Object.new
#     expect { @cr.add_receiver(receiver, :command) }.to raise_error
#
#   end
#
# end
