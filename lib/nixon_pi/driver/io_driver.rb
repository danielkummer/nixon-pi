module NixonPi
  module Driver
    class IoDriver
      include Logging
      include Driver

      def initialize(ports)
        @ports = ports
        log.debug "Initialized io driver for ports #{@ports}"
      end

      def write(value, port = nil)
        if port
          client.io_write(@ports[port], value)
        else
          @ports.each { |p| client.io_write(p, value) }
        end
      end

      def read(port)
        client.io_read(@ports[port])
      end
    end
  end

end