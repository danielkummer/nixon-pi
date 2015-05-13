
module NixonPi
  class HardwareInfo
    include InformationHolder
    include Logging

    def handle_info_request(about)
      ret = {}
      case about.to_sym
        when :io_carad
          ret = { io_card: AbioCardClient.instance.info }
        when :pi
          # TODO: get information about cpu ram hdd etc..., (maybe distro)
          ret = {}
        when :network
          #todo test!
          ret = { network: NetworkInfo.info}
        else
          log.error "No information about #{about}"
      end
      ret
    end
  end
end
