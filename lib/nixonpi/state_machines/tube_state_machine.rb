require 'state_machine'
require 'festivaltts4r'


require_relative '../drivers/tube_driver'
require 'active_support/inflector'
require_relative '../animations/animation'
require_relative '../animations/tube/switch_numbers_animation'
require_relative '../animations/tube/single_fly_in_animation'
require_relative 'handler_state_machine'


module NixonPi
  class TubeStateMachine < HandlerStateMachine

    register_as :tubes

    def after_create
      register_driver NixonPi::TubeDriver
    end

    state_machine :initial => :time do

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

      event :test do
        transition all => :test
      end

      state :time do
        def write
          tubes_count = Settings.in12a_tubes.count

          format = current_state_parameters[:time_format]

          if format.nil? or format.size > tubes_count
            format = Settings.default_time_format
          end

          time = Time.now.strftime(format).rjust(tubes_count, ' ')
          driver.write(time) unless time.nil?
          current_state_parameters[:last_value] = time
        end
      end

      state :free_value do
        def write
          value = current_state_parameters[:value]
          driver.write(value)
          current_state_parameters[:last_value] = value
        end
      end

      state :animation do
        def write
          name, options = current_state_parameters[:animation_name], current_state_parameters[:animation_options]
          options ||= {}
          start_value = current_state_parameters[:last_value]
          NixonPi::Animations::Animation.create(name.to_sym, options).run(start_value)
          self.send(current_state_parameters[:last_state]) #go back to old state again and do whatever was done before
        end
      end

      state :test do
        def write
          unless current_state_parameters[:test_done]
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
            self.fire_state_event(current_state_parameters[:last_state])
          end
        end
      end
    end
  end
end
