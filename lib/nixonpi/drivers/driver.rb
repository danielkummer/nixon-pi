require 'singleton'

require_relative '../logging/logging'
require_relative '../factory'
require_relative '../configurations/settings'

module NixonPi
  module Driver
    include Logging


    def client
      @client ||= NixonPi::AbioCardClient.instance
    end

    def write(params)
      raise NotImplementedError
    end

  end
end
