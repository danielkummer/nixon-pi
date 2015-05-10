require 'socket'


module NixonPi
  class NetworkInfo

    class << self
      def info
        res = Socket.ip_address_list.select{|intf| intf.ipv4? and !intf.ipv4_loopback? and !intf.ipv4_multicast?}.collect{|addr| addr.ip_address }
        res
      end
    end

  end
end
