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
require_relative 'nixieberry/handlers/bar_handler_sate_machine'
require_relative 'nixieberry/handlers/tube_handler_state_machine'
require_relative 'nixieberry/handlers/lamp_handler_state_machine'
require_relative 'nixieberry/animations/animation'
require_relative 'nixieberry/web/sinatra_server'

module NixieBerry
  class NixieService
    include Logging

    def initialize
      log.info "Initializing Service.."
      @tsm = NixieBerry::TubeHandlerStateMachine.instance
      @bsm = NixieBerry::BarHandlerStateMachine.instance
      @lsm = NixieBerry::LampHandlerStateMachine.instance
      @server = RESTServer
    end

    ##
    # Run service run
    def run
      [:INT, :TERM].each do |sig|
        trap(sig) do
          log.info "Shutting down..."
          log.info "Bye ;)"
          exit(0)
        end
      end


      log.info "Start running..."
      #todo there must be a better way to control creating and shutting down!
      #unless ENV['NIXIE_BERRY_ENVIRONMENT'] = 'test'
=begin
        t = Thread.new do
          @server.run!
        end
      #end
=end
      loop do
        @tsm.handle
        @bsm.handle
        @lsm.handle
        sleep 0.3 #tacted at 30ms, adjust if necessary
      end

    end
  end
end

