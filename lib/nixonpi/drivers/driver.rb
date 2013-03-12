require_relative '../logging/logging'

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
