require_relative 'spec_helper'
require_relative 'support/active_record'
require_relative '../web/models'

describe Command do


  context "tube commands" do
    it "should return valid for a valid a tube command" do
      tc = Command.new(state_machine: :tube, state: :free_value, value: "1234 1234")
      tc.valid?.should be true
                             #     model.should have(1).error_on(:attribute)

          #tc.errors[:base].should includ
    end

    it "should fail on a non integer or whitespace value" do

    end

  end
end