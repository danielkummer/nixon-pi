require_relative 'spec_helper'
require_relative 'support/active_record'
require_relative '../web/models'

describe Command do
  context "tube commands" do

    it "should fail on unsupported states" do
      @cmd = Command.new(state_machine: :tubes, state: :some_state)
      @cmd.valid?.should be false
      @cmd.should have(1).error_on(:state)
    end

    context "free value" do
      before :each do
        @cmd = Command.new(state_machine: :tubes, state: :free_value)
      end

      it "should return valid for a valid a tube command" do
        @cmd.value="1234 1234"
        @cmd.valid?.should be true
        #     model.should have(1).error_on(:attribute)

        #tc.errors[:base].should includ
      end
      it "should fail on a non integer or whitespace value" do
        @cmd.value="notvalid"
        @cmd.valid?.should be false
        @cmd.should have(1).error_on(:value)
      end

      it "should fail if the output value is longer than the number of tubes" do
        @cmd.value="1234567890123"
        @cmd.valid?.should be false
        @cmd.should have(1).error_on(:value)
      end

    end

    context "time" do
      before :each do
        @cmd = Command.new(state_machine: :tubes, state: :time)
      end

      it "should throw an error on an invalid time string" do
        @cmd.value="invalid"
        @cmd.valid?.should be false
        @cmd.should have(1).error_on(:value)
      end

      it "should correctly validate a time string" do
        @cmd.value = "%H%M%S"
        @cmd.valid?.should be true
      end

    end

    context "animation" do

      before :each do
        @cmd = Command.new(state_machine: :tubes, state: :animation)
      end

      it "should require an animation name" do
        @cmd.animation_name = ""
        @cmd.valid?.should be false
        @cmd.should have(1).error_on(:animation_name)
      end

      it "should fail if non json options are provided" do
        @cmd.options = "THIS IS NO JSON"
        @cmd.valid?.should be false
        @cmd.should have(1).error_on(:options)
      end

      it "should succeed if an animation name and json options are provieded" do
        @cmd.animation_name = "animation"
        @cmd.options = '{"json": "string"}'
        @cmd.valid?.should be true
      end
    end

    context "countdown" do

      before :each do
        @cmd = Command.new(state_machine: :tubes, state: :countdown)
      end

      it "should only allow a valid time string" do
        @cmd.value="invalid"
        @cmd.valid?.should be false
        @cmd.should have(1).error_on(:value)
      end

      it "should suceed when passing a parseable time string" do
        @cmd.value="1h5m"
        @cmd.valid?.should be true
      end
    end

    context "meeting ticker" do
      before :each do
        @cmd = Command.new(state_machine: :tubes, state: :meeting_ticker)
      end

      it "should fail on an ivalid meeting ticker input" do
        @cmd.value="invalid"
        @cmd.valid?.should be false
        @cmd.should have(1).error_on(:value)
      end

      it "should allow a valid meeting ticker input" do
        @cmd.value="200:300"
        @cmd.valid?.should be true
      end

    end

  end

  context "bar commands" do

    it "should fail on unsupported states" do
      @cmd = Command.new(state_machine: :bars, state: :some_state)
      @cmd.valid?.should be false
      @cmd.should have(1).error_on(:state)
    end

    it "should fail on unsupported states" do
      @cmd = Command.new(state_machine: :bars, state: :some_state)
      @cmd.valid?.should be false
      @cmd.should have(1).error_on(:state)
    end

    context "free value" do
      before :each do
        @cmd = Command.new(state_machine: :bars, state: :free_value)
      end

      it "should only allow values in supported range" do
        @cmd.value = "125"
        @cmd.valid?.should be true
        @cmd.value = "255"
        @cmd.valid?.should be true
        @cmd.value = "500"
        @cmd.valid?.should be false
      end

    end

    context "animation" do

      before :each do
        @cmd = Command.new(state_machine: :bars, state: :animation)
      end

      it "should require an animation name" do
        @cmd.animation_name = ""
        @cmd.valid?.should be false
        @cmd.should have(1).error_on(:animation_name)
      end

      it "should fail if non json options are provided" do
        @cmd.options = "THIS IS NO JSON"
        @cmd.valid?.should be false
        @cmd.should have(1).error_on(:options)
      end

      it "should succeed if an animation name and json options are provieded" do
        @cmd.animation_name = "animation"
        @cmd.options = '{"json": "string"}'
        @cmd.valid?.should be true
      end
    end
  end

  context "lamp commands" do

    it "should fail on unsupported states" do
      @cmd = Command.new(state_machine: :lamps, state: :some_state)
      @cmd.valid?.should be false
      @cmd.should have(1).error_on(:state)
    end

    context "free value" do
      before :each do
        @cmd = Command.new(state_machine: :lamps, state: :free_value)
      end

      it "should only allow values in supported range" do
        @cmd.value = "0"
        @cmd.valid?.should be true
        @cmd.value = "1"
        @cmd.valid?.should be true
        @cmd.value = "500"
        @cmd.valid?.should be false
      end

    end
  end

  context "power commands" do

    context "free value" do
      before :each do
        @cmd = Command.new(state_machine: :power, state: :free_value)
      end

      it "should only allow values in supported range" do
        @cmd.value = "0"
        @cmd.valid?.should be true
        @cmd.value = "1"
        @cmd.valid?.should be true
        @cmd.value = "500"
        @cmd.valid?.should be false
      end
    end
  end


  context "say commands" do
    context "free value" do
      before :each do
        @cmd = Command.new(state_machine: :say, state: :free_value)
      end

      it "should throw an error on an empty value" do
        @cmd.value = ""
        @cmd.valid?.should be false
        @cmd.should have(1).error_on(:value)
      end

      it "should be valid if the value is not empty" do
        @cmd.value = "hello there"
        @cmd.valid?.should be true
      end
    end
  end
end