require 'state_machine'
require 'festivaltts4r'
require 'singleton'


require_relative '../drivers/tube_driver'
require 'active_support/inflector'
require_relative '../animations/animation'
require_relative '../animations/tube/switch_numbers_animation'
require_relative '../animations/tube/single_fly_in_animation'
require_relative 'handler_state_machine'


module NixieBerry
  class TubeHandlerStateMachine < HandlerStateMachine
    include Singleton

    register_driver NixieBerry::TubeDriver.instance
    register_queue_name :tubes

    state_machine :initial => :display_time do

      around_transition do |object, transition, block|
        handle_around_transition(object, transition, block)
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
        def write
          tubes_count = Settings.in12a_tubes.count
          format = @current_state_parameters[:time_format]

          if format.nil? or format.size > tubes_count
            #log.debug "Using default time format, 6 tubes needed"
            format = Settings.default_time_format
          end

          time = Time.now.strftime(format).rjust(tubes_count, ' ')
          @driver.write(time) unless time == @current_state_parameters[:last_value] or time.nil?
          #todo might be unneccessary
          @current_state_parameters[:last_value] = time
        end
      end

      state :display_free_value do
        def write
          value = @current_state_parameters[:value]
          unless value == @current_state_parameters[:last_value] or value.nil?
            @driver.write(value)
            @current_state_parameters[:last_value] = value
          end
        end
      end

      #todo bug here - the thread could still be running inside the animation run method!

      state :display_tube_animation do
        def write
          animation_name = @current_state_parameters[:animation_name]
          animation_options = @current_state_parameters[:animation_options]
          animation_options ||= {}
          start_value = @current_state_parameters[:last_value]
          NixieBerry::Animations::Animation.create(animation_name.to_sym, animation_options).run(start_value)
          self.send(@current_state_parameters[:last_state]) #go back to old state again and do whatever was done before
        end
      end

      state :test do
        def write
          unless @current_state_parameters[:test_done]
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
            NixieBerry::Animations::Animation.create(:switch_numbers).run("    12345678")
            NixieBerry::Animations::Animation.create(:single_fly_in).run("    12345678")

            puts "Benchmark write 2-pair strings from 00 to 99:"
            puts bm
            self.fire_state_event(@current_state_parameters[:last_state])
          end

        end
      end
    end
  end
end