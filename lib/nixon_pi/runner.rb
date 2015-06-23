require 'thread'

module NixonPi
  ##
  # Main class for the service, it runs the service inside a thread and handles graceful service shutdown on interrupts
  class Runner
    include Logging

    def initialize
      @service = NixonPi::NixieService.new

    end

    def shutdown!
      @udp_server.exit
      Thread.new do
        @service.shutdown
        log.info 'Shutdown complete - cya next time :)'
      end
    end

    def run(_opts = {})
      trap('TERM') do
        shutdown!
      end
      trap('INT') do
        shutdown!
      end
      trap('QUIT') do
        shutdown!
      end

      log.info 'Starting udp server'
      @udp_server = NixonPi::UDPPing.start_service_announcer($upd_port) do |client_msg, client_ip|
        {client_ip: client_ip, ip_addresses: OSInfo.network }
      end

      @service.run!
      @udp_server.join
    end
  end
end
