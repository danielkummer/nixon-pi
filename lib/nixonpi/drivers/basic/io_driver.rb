require_relative '../driver'
require_relative '../../logging/logging'

module NixonPi
  class IoDriver
    include Driver
    include Logging

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
