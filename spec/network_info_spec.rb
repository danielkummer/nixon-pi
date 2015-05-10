require_relative 'spec_helper'
require_relative '../lib/nixonpi/support/network_info'

describe NixonPi::NetworkInfo do

  it 'returns network information' do
    expect(NixonPi::NetworkInfo.info).not_to be_empty
  end

end