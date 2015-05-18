
# TODO: refactor to parent class
module NixonPi
  module Driver
    ##
    # Returns an AbioCardClient instance
    # @return [AbioCardClient] client instance
    def client
      @client ||= NixonPi::AbioCardClient.instance
    end

    def write(_params)
      fail NotImplementedError
    end
  end
end
