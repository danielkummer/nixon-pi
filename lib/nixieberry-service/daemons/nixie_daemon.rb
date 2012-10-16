require 'rubygems'
require 'json'

require 'logger'
require 'benchmark'

require_relative '../../../lib/nixieberry-service'

CONFIG_ENV = :development


module NixieBerry
  class NixieDaemon
  include NixieLogger


    def initialize
      @nixie = NixieBerry::TubeDriver.instance
      @state_machine = NixieBerry::TubeHandlerStateMachine.new
      @controlconfig = NixieBerry::ControlConfiguration.instance
      @bar_handler = NixieBerry::BarHandler.new
    end

    def run
      loop do
        @controlconfig.update_from_redis

        unless @controlconfig[:mode].to_sym == :test
          @state_machine.fire_state_event(@controlconfig[:mode].to_sym) unless @controlconfig[:mode] == @state_machine.state
          @state_machine.write_to_tubes

          @bar_handler.write_to_bars
        else
          test_run
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


      puts "Benchmark write 2-pair strings from 00 to 99:"
      puts bm

    end
  end
end

server = NixieBerry::NixieDaemon.new
server.run
#server.test_run
