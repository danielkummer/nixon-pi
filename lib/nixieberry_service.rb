require 'rubygems'
require 'json'
require 'logger'
require 'benchmark'
#require 'profiler'

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
require_relative 'nixieberry/web/web_server'

module NixieBerry
  class NixieService
    include Logging

    def initialize
      log.info "Initializing Service.."
      @tsm = NixieBerry::HandlerStateMachine.create(:tubes)
      @bsm = NixieBerry::HandlerStateMachine.create(:bars)
      @lsm = NixieBerry::HandlerStateMachine.create(:lamps)
      @server = WebServer
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

      t = Thread.new do
        @server.run!
      end
      #end

      threads = []
      threads << Thread.new do
        loop do
          #puts "tsm active"
          @tsm.handle
          sleep 0.3 #tacted at 300ms, adjust if necessary
        end
      end

      threads << Thread.new do
        loop do
          #puts "bsm active"
          @bsm.handle
          sleep 0.3 #tacted at 300ms, adjust if necessary
        end
      end

      threads << Thread.new do
        loop do
          #puts "lsm active"
          @lsm.handle
          sleep 0.3 #tacted at 300ms, adjust if necessary
        end
      end

      threads.each {|thread| thread.join }

    end
  end
end

