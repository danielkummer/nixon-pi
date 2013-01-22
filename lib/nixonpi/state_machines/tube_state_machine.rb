require 'state_machine'
require 'festivaltts4r'


require_relative '../drivers/tube_driver'
require 'active_support/inflector'
require_relative '../animations/animation'
require_relative '../animations/tube/switch_numbers_animation'
require_relative '../animations/tube/single_fly_in_animation'
require_relative 'handler_state_machine'
require_relative '../command_queue'
require_relative '../logging/logging'


module NixonPi
  class TubeStateMachine < HandlerStateMachine
    include Logging

    register_as :tubes
    accepted_commands :state, :value, :time_format, :animation_name, :options, :initial_mode



    def initialize()
      super()
      register_driver NixonPi::TubeDriver
    end

    state_machine :initial => :startup do


      around_transition do |object, transition, block|
        HandlerStateMachine.handle_around_transition(object, transition, block)
      end

      event :free_value do
        transition all => :free_value
      end

      event :time do
        transition all => :time
      end

      event :animation do
        transition all => :animation
      end

      event :countdown do
        transition all => :countdown
      end

      event :run_test do
        transition all => :run_test
      end

      state :startup do
        def write
          params[:animation_name] = "single_fly_in"
          params[:options] = {}
          params[:last_value] = "0000"
          self.fire_state_event(:animation)
          if params[:initial_state].nil?
            params[:last_state] = :time
          else
            params[:last_state] = params[:initial_state]
          end
        end
      end

      state :time do
        def write
          tubes_count = Settings.in12a_tubes.count
          format = params[:time_format]
          if format.nil? or format.size > tubes_count
            format = Settings.default_time_format
          end

          now = Time.now
          params[:last_time] = now if params[:last_time].nil?

          formatted_time = now.strftime(format)
          formatted_date = now.strftime(Settings.default_date_format)


          if now.min == 0 and now.min != params[:last_time].min
            NixonPi::Animations::Animation.create(:single_fly_in).run(formatted_time)
          elsif now.hour == 0 and now.hour != params[:last_time].hour
            NixonPi::Animations::Animation.create(:single_fly_in).run(formatted_time)
          elsif now.min % 15 == 0 and now.sec <= 10
            driver.write(formatted_date.rjust(tubes_count, ' '))
          else
            driver.write(formatted_time.rjust(tubes_count, ' '))
          end

          params[:last_value] = formatted_time
          params[:last_time] = now

        end
      end

      state :free_value do
        def write
          value = params[:value]
          driver.write(value)
          params[:last_value] = value
        end
      end

      state :animation do
        def write
          name, options = params[:animation_name], params[:options]
          options ||= {}
          start_value = params[:last_value]
          NixonPi::Animations::Animation.create(name.to_sym, options).run(start_value)
          self.fire_state_event(params[:last_state]) #go back to old state again and do whatever was done before
        end
      end

      #todo refactor
      state :countdown do
        require 'chronic_duration'

        def write
          seconds_to_go = params[:value].to_i
          current_time = Time.now

          if params[:target_time].nil?
            params[:target_time] = Time.now + seconds_to_go.seconds
          end
          target_time = params[:target_time]

          next_value = target_time - current_time

          if seconds_to_go > 0 and seconds_to_go != next_value

            output = ChronicDuration.output(seconds_to_go, :format => :chrono).gsub(/:/, ' ')
            log.debug "write countdown: #{output}"
            driver.write(output)
          end

          params[:value] = next_value

          if current_time >= params[:target_time]
            log.debug "end of countdown"
            options = params[:options]
            unless options.nil?
              if options.is_a? Hash
                options.keys.each do |k, o|
                  case k.to_sym
                    when :say
                      CommandQueue.enqueue(:say, {value: o})
                    when :state
                      self.fire_state_event(params[:o])
                    else
                      log.debug "option unknown"
                  end
                end
              end
            end
            self.fire_state_event(params[:last_state])
          end
        end
      end

      state :run_test do
        def write
          unless params[:test_done]
            number_of_digits = 12
            test_data = Array.new
            9.times do |time|
              test_data << "#{time}" * number_of_digits
            end
            i = 0
            bm = Benchmark.measure do
              test_data.each do |val|
                driver.write(val)
                #sleep 0.1
                i += 1
              end
            end

            #simple animation
            NixonPi::Animations::Animation.create(:switch_numbers).run("    12345678")
            NixonPi::Animations::Animation.create(:single_fly_in).run("    12345678")

            puts "Benchmark write 2-pair strings from 00 to 99:"
            puts bm
            self.fire_state_event(params[:last_state])
          end
        end
      end
    end
  end
end
