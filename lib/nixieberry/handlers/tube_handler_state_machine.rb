require 'state_machine'
require 'festivaltts4r'

require_relative '../logging/logging'
require_relative '../drivers/tube_driver'
require_relative '../configurations/state_hash'
require 'active_support/inflector'
require_relative '../command_queue'
require_relative '../animations/animation'
require_relative '../animations/tube/switch_numbers_animation'
require_relative '../animations/tube/single_fly_in_animation'


module NixieBerry
  class TubeHandlerStateMachine
    include Logging
    include CommandQueue

    attr_accessor :state_params

    def initialize
      @old_values = {}
      @driver = NixieBerry::TubeDriver.instance


      @state_params = StateHash.new
      @state_params[:last_state] = nil

      super() # NOTE: This *must* be called, otherwise states won't get initialized
    end


    def set_state_params(params)
      @state_params.merge!(params) unless params.nil?
    end

    def state_info
      @state_params
    end

    def handle_command_queue
      tube_queue = queue(:tubes)
      unless tube_queue.empty?
        current_command = tube_queue.pop
        command_type = current_command.type
        params = current_command.params
        log.warn("Got command: #{command_type} with params #{params}")
        case command_type.to_sym
          when :mode
            self.fire_state_event(params.to_sym) if self.state != params
          when :value
            self.set_state_params(params)
        end
      end
    end

    state_machine :initial => :display_time do

      around_transition do |object, transition, block|
        object.log.debug "doing transition  #{transition.event} from state: #{object.state}"
        object.state_params[:last_state] = object.state
        #transition.event.to_s.humanize.to_speech #say the current state transition
        block.call
        object.state_params[:state] = object.state
        object.log.debug "new state: #{object.state}"
      end

      event :display_free_value do
        transition all => :display_free_value
      end

      event :display_time do
        transition all => :display_time
      end

      event :display_tube_animation do
        transition all => :display_tube_animation
      end

      event :test do
        transition all => :test
      end

      state :display_time do
        def write_to_tubes
          tubes_count = Settings.in12a_tubes.count
          format = @state_params[:time_format]

          if format.nil? or format.size > tubes_count
            #log.debug "Using default time format, 6 tubes needed"
            format = Settings.default_time_format
          end

          time = Time.now.strftime(format).rjust(tubes_count, ' ')
          @driver.write(time) unless time == @state_params[:last_value] or time.nil?
          #todo might be unneccessary
          @state_params[:last_value] = time
        end
      end

      state :display_free_value do
        def write_to_tubes
          value = @state_params[:value]
          unless value == @last_written_free_value or value.nil?
            @driver.write(value)
            @last_written_free_value = value
          end
        end
      end

      state :display_tube_animation do
        def write_to_tubes
          animation_name = @state_params[:animation_name]
          animation_options = @state_params[:animation_options]
          animation_options ||= {}
          Animation.create(animation_name.to_sym, animation_options).run
          self.send(@state_params[:last_state]) #go back to old state again and do whatever was done before
        end
      end

      state :test do
        def write_to_tubes
          unless @state_params[:test_done]
            number_of_digits = 12
            test_data = Array.new
            9.times do |time|
              test_data << "#{time}" * number_of_digits
            end
            i = 0
            bm = Benchmark.measure do
              test_data.each do |val|
                @driver.write(val)
                #sleep 0.1
                i += 1
              end
            end

            #simple animation
            NixieBerry::Animations::Animation.create(:switch_numbers).run("12345678")
            NixieBerry::Animations::Animation.create(:single_fly_in).run("12345678")

            puts "Benchmark write 2-pair strings from 00 to 99:"
            puts bm
            self.fire_state_event(@state_params[:last_state])
          end

        end
      end

    end
  end
end