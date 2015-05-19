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

      @service.run!
    end
  end
end
