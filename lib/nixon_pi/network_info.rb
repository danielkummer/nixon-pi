require 'socket'

module NixonPi
  class NetworkInfo
    class << self
      ##
      # Get local ip addresses (except loopback)
      #
      # @return [Array] local ip addresses
      def info
        res = Socket.ip_address_list.select { |intf| intf.ipv4? && !intf.ipv4_loopback? && !intf.ipv4_multicast? }.collect(&:ip_address)
        res
      end
    end
  end
end
