require_relative 'spec_helper'
require_relative '../lib/nixieberry/configurations/state_hash'

describe NixieBerry::LockingHash do

  it "should create a new lockinghash instance" do
    h = NixieBerry::LockingHash.new
    h.should_not be_nil
  end

  it "should create an accessor method for a created hash key" do
    h = NixieBerry::LockingHash.new
    h['test1'] = "hello world"
    h.test1.should eq "hello world"
    h['another'] = "hello again"
    h.another.should eq "hello again"
    h[:hello] = "world"
    h.hello.should eq("world")
  end

end