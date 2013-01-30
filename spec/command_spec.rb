require_relative 'spec_helper'
require_relative 'support/active_record'
require_relative '../web/models'

describe Command do


  context "tube commands" do
    context "free value" do


      before :each do
        @tc = Command.new(state_machine: :tubes, state: :free_value)
      end

      it "should return valid for a valid a tube command" do
        @tc.value="1234 1234"
        @tc.valid?.should be true
        #     model.should have(1).error_on(:attribute)

        #tc.errors[:base].should includ
      end
      it "should fail on a non integer or whitespace value" do
        @tc.value="notvalid"
        @tc.valid?.should be false
        @tc.should have(1).error_on(:value)
      end

      it "should fail if the output value is longer than the number of tubes" do
        @tc.value="1234567890123"
        @tc.valid?.should be false
        @tc.should have(1).error_on(:value)
      end

    end
    context "time" do
      before :each do
        @tc = Command.new(state_machine: :tubes, state: :time)
      end

      it "should only allow valid time string" do
        fail
      end

    end

    context "animation" do
      it "should require an animation name" do
        fail
      end

      it "should fail if non json options are provided" do
        fail
      end
    end

    context "countdown" do
      it "should only allow a valid time string" do
        fail
      end
    end

  end
end