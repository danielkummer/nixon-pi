
module NixonPi
  class Runner
    include Logging

    def initialize
      @service = NixonPi::NixieService.new
    end

    def shutdown!
      Thread.new do
        @service.shutdown
        log.info 'Shutting down...'
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
