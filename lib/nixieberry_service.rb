require 'rubygems'
require 'json'
require 'logger'
require 'benchmark'
require 'profiler'

require_relative 'nixieberry/version'
require_relative 'nixieberry/animation_queue'
require_relative 'nixieberry/delegators/multi_delegator'
require_relative 'nixieberry/configurations/configuration'
require_relative 'nixieberry/configurations/control'
require_relative 'nixieberry/logging/logging'
require_relative 'nixieberry/client/abio_card_client'
require_relative 'nixieberry/drivers/tube_driver'
require_relative 'nixieberry/drivers/lamp_driver'
require_relative 'nixieberry/drivers/bar_graph_driver'
require_relative 'nixieberry/handlers/bar_handler'
require_relative 'nixieberry/handlers/tube_handler_state_machine'
require_relative 'nixieberry/animations/animation'


module NixieBerry

  class Service
    include Logging


    def initialize
      @nixie = NixieBerry::TubeDriver.instance
      @state_machine = NixieBerry::TubeHandlerStateMachine.new
      @controlconfig = NixieBerry::Control.instance
      @bar_handler = NixieBerry::BarHandler.new
    end

    ##
    # Run the nixie berry service, controlled via redis
    def run
      loop do
        @controlconfig.update_from_redis

        if @controlconfig[:mode].to_sym == :test
          test_run
        else
          @state_machine.fire_state_event(@controlconfig[:mode].to_sym) unless @controlconfig[:mode] == @state_machine.state
          @state_machine.write_to_tubes
          @bar_handler.write_to_bars

        end

        #todo remove the sleep
        sleep 1 #sleep one sec
      end
    end

    def test_run
      number_of_digits = 12
      9.times do |time|
        test_data << "#{time}" * number_of_digits
      end
      i = 0
      bm = Benchmark.measure do
        test_data.each do |val|
          @nixie.write(val)
          sleep 2
          i += 1
        end
      end

      #simple animation
      Animation.create(:switch_numbers, value: "12345678").run

      puts "Benchmark write 2-pair strings from 00 to 99:"
      puts bm

    end
  end
end

