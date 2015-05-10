require_relative 'spec_helper'
require_relative '../lib/blank_monkeypatch'

describe Object do
  it 'should have the blank? method' do
    Object.new.should respond_to :blank?
  end

  it 'should be blank if nil' do
    o = nil
    o.blank?.should be true
  end

  context 'a string' do
    it 'should be blank if empty' do
      ''.blank?.should be true
    end

    it 'should not be blank if not empty' do
      'not blank'.blank?.should be false
    end
  end
end
