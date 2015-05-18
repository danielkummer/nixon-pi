
module NixonPi
  class HardwareInfo
    include InfoResponder
    include Logging

    def handle_info_request(about)
      ret = {}
      case about.to_sym
        when :io_card
          ret = { io_card: AbioCardClient.instance.info }
        when :pi
          # TODO: get information about cpu ram hdd etc..., (maybe distro)
          ret = {}
        when :network
          # TODO: test!
          ret = { network: OSInfo.network }
        else
          log.error "No information about #{about}"
      end
      ret
    end
  end
end
