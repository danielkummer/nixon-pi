require 'singleton'

require_relative '../logging/logging'
require_relative '../configurations/settings'

module NixieBerry
  class Driver
    include Logging

    def initialize
      @client = NixieBerry::AbioCardClient.instance
    end

    def write(params)
      raise NotImplementedError
    end

  end
end
