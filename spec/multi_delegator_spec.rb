require_relative 'spec_helper'
require_relative '../lib/nixonpi/delegators/multi_delegator'

class Entry
  def initialize(target)
    @target = target
  end

  def write(arg)
    @target.write(arg)
  end
end

class DelegateWasOk < StandardError; end

class Receiver
  def self.write(_out)
    fail DelegateWasOk
  end
end

describe NixonPi::MultiDelegator do
  before :each do
    @entry = Entry.new NixonPi::MultiDelegator.delegate(:write).to(Receiver)
  end

  it 'should delegate to multiple targets' do
    expect do
      @entry.write('Hello')
    end.to raise_error DelegateWasOk
  end
end
