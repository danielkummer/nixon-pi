require_relative '../logging/logging'

module NixonPi
  module Driver
    include Logging

    ##
    # Returns an AbioCardClient instance
    # @return [AbioCardClient] client instance
    def client
      @client ||= NixonPi::AbioCardClient.instance
    end

    def write(params)
      raise NotImplementedError
    end

  end
end
