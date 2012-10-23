require 'rubygems'
require 'json'

require 'logger'
require 'benchmark'

require_relative 'nixieberry-service/version'
require_relative 'nixieberry-service/animation_queue'
require_relative 'nixieberry-service/delegators/multi_delegator'
require_relative 'nixieberry-service/configurations/control_configuration'
require_relative 'nixieberry-service/configurations/nixie_config'
require_relative 'nixieberry-service/logging/nixie_logger'
require_relative 'nixieberry-service/client/abio_card_client'
require_relative 'nixieberry-service/drivers/tube_driver'
require_relative 'nixieberry-service/drivers/neon_driver'
require_relative 'nixieberry-service/drivers/bar_driver'
require_relative 'nixieberry-service/handlers/bar_handler'
require_relative 'nixieberry-service/handlers/tube_handler_state_machine'
require_relative 'nixieberry-service/animations/tube_animation'


CONFIG_ENV = :development

module NixieBerry


  ## Exceptions
  class RetryError < StandardError;
  end

  class Service
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
      TubeAnimation.create(:switch_numbers, value: "12345678").run

      puts "Benchmark write 2-pair strings from 00 to 99:"
      puts bm

    end
  end
end

server = NixieBerry::Service.new
server.run

