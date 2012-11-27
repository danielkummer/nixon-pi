require 'rubygems'

require_relative 'nixonpi/version'
require_relative 'nixonpi/delegators/multi_delegator'
require_relative 'nixonpi/configurations/state_hash'
require_relative 'nixonpi/logging/logging'
require_relative 'nixonpi/client/abio_card_client'
require_relative 'nixonpi/drivers/tube_driver'
require_relative 'nixonpi/drivers/lamp_driver'
require_relative 'nixonpi/drivers/bar_graph_driver'
require_relative 'nixonpi/state_machines/bar_sate_machine'
require_relative 'nixonpi/state_machines/tube_state_machine'
require_relative 'nixonpi/state_machines/lamp_state_machine'
require_relative 'nixonpi/animations/animation'
require_relative 'nixonpi/web/web_server'
require_relative 'nixonpi/state_machines/machine_manager'

module NixonPi
  class NixieService
    include Logging

    def initialize
      log.info "Initializing Service.."
      NixonPi::MachineManager.add_state_machines(:tubes, :bars, :lamps)
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

        NixonPi::MachineManager.start_state_machines
        NixonPi::MachineManager.on_exit
      rescue Interrupt
        puts "Exiting..."
      end

    end
  end
end

