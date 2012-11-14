require 'rubygems'
require 'json'
require 'logger'
require 'benchmark'
require 'profiler'

require_relative 'nixieberry/version'
require_relative 'nixieberry/delegators/multi_delegator'
require_relative 'nixieberry/configurations/state_hash'
require_relative 'nixieberry/logging/logging'
require_relative 'nixieberry/client/abio_card_client'
require_relative 'nixieberry/drivers/tube_driver'
require_relative 'nixieberry/drivers/lamp_driver'
require_relative 'nixieberry/drivers/bar_graph_driver'
require_relative 'nixieberry/handlers/bar_handler'
require_relative 'nixieberry/handlers/tube_handler_state_machine'
require_relative 'nixieberry/animations/animation'
require_relative 'nixieberry/web/sinatra_server'

module NixieBerry
  class NixieService
    include Logging

    def initialize
      log.info "Initializing Service.."
      @nixie = NixieBerry::TubeDriver.instance
      @tsm = NixieBerry::TubeHandlerStateMachine.instance
      @bar_handler = NixieBerry::BarHandler.new
    end

    ##
    # Run the nixie berry service, controlled via redis
    def run
      log.info "Start running..."
      t = Thread.new do
        RESTServer.run!
      end


      #todo not working!!
      [:INT, :TERM, :EXIT].each do |sig|
        trap(sig) do
          log.info "Shutting down..."
          t.exit
          log.info "Bye ;)"
          exit(0)
        end
      end


      loop do
        @tsm.handle_command_queue
        @tsm.write_to_tubes
      end


      # @bar_handler.write_to_bars

    end
  end
end

