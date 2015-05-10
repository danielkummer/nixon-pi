require_relative 'spec_helper'
require_relative '../lib/nixonpi/client/retryable'
require_relative '../lib/nixonpi/logging/logging'

class Retry
  include NixonPi::Retryable
  include NixonPi::Logging
end

describe NixonPi::Retryable do
  before :each do
    @retry = Retry.new
  end

  it 'should not raise an exception if less than specified retries' do
    @times = 0
    expect do
      @retry.retryable do
        @times += 1
        fail Exception if @times < 5
      end
    end.not_to raise_error
  end

  it 'should raise an exception if more than specified retries' do
    expect do
      @retry.retryable do
        fail Exception
      end
    end.to raise_error NixonPi::RetryError
  end
end
