module NixonPi
  module InformationHolder
    # Return an information hash based on the passed parameter
    # @param [Symbol] about
    # @return [Hash] hash containing requested information
    def handle_info_request(about)
      raise NotImplementedError
    end
  end
end