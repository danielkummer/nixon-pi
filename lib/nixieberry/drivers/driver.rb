require 'singleton'

require_relative '../logging/logging'
require_relative '../configurations/settings'

module NixieBerry
  module Driver
    include Logging

    def client
      @client ||= NixieBerry::AbioCardClient.instance
    end

    def write(params)
      raise NotImplementedError
    end

  end
end
