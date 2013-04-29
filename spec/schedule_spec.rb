require_relative 'spec_helper'
require_relative 'support/active_record'
require_relative '../web/models'

describe Schedule do

  it "should throw an error on an unsupported timing method" do
    @sched = Schedule.new(method: :thismethoddoesntexist)
    @sched.valid?.should be false
    @sched.should have(1).error_on(:method)
  end

  it "should validate the presence of the required attributes" do
    @sched = Schedule.new
    @sched.valid?.should be false
    @sched.should have(1).error_on(:target)
    @sched.should have(1).error_on(:method)
    @sched.should have(1).error_on(:timing)
    @sched.should have(1).error_on(:command)

  end

  context "in" do

    before :each do
      @sched = Schedule.new(method: :in, target: :target, command: "some command")
    end

    it "should validate a rufus parseable time string" do
      @sched.timing = "10m"
      @sched.valid?.should be true
    end

    it "should reject an unparseable time string" do
      @sched.timing = "dunotnow"
      @sched.valid?.should be false
      @sched.should have(1).error_on(:timing)
    end

  end

  context "every" do
    before :each do
      @sched = Schedule.new(method: :every, target: :target, command: "some command")
    end

    it "should validate a rufus parseable time string" do
      @sched.timing = "10m"
      @sched.valid?.should be true
    end

    it "should reject an unparseable time string" do
      @sched.timing = "dunotnow"
      @sched.valid?.should be false
      @sched.should have(1).error_on(:timing)
    end
  end

  context "at" do

    before :each do
      @sched = Schedule.new(method: :at, target: :target, command: "some command")
    end

    it "should validate a rufus parseable time string" do
      @sched.timing = (Time.now + 1.day).to_s
      @sched.valid?.should be true
      @sched
    end

    it "should reject an unparseable time string" do
      @sched.timing = "dunotnow"
      @sched.valid?.should be false
      @sched.should have(1).error_on(:timing)
    end

  end

  context "cron" do

  end

end