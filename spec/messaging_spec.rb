require_relative 'spec_helper'

require_relative '../lib/nixonpi/messaging/command_receiver'

# describe NixonPi::Messaging::CommandReceiver do
#
#   before :each do
#     @cr = NixonPi::Messaging::CommandReceiver.new
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
