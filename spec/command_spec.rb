require_relative 'spec_helper'
require_relative 'support/active_record'
require_relative '../web/models'

describe Command do
  it { should validate_presence_of :state_machine }
end



