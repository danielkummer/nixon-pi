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
require_relative 'nixieberry/handlers/machine_manager'

module NixieBerry
  class NixieService
    include Logging

    def initialize
      log.info "Initializing Service.."
      NixieBerry::MachineManager.add_state_machines(:tubes, :bars, :lamps)
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

      begin
        log.info "Start running..."
        #todo there must be a better way to control creating and shutting down!
        #unless $environment  = 'test'

        t = Thread.new do
          @server.run!
        end
        #end

        NixieBerry::MachineManager.start_state_machines
        NixieBerry::MachineManager.on_exit
      rescue Interrupt
        puts "Exiting..."
      end

    end
  end
end

