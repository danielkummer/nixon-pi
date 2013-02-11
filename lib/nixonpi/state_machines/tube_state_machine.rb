require 'state_machine'

require_relative '../drivers/tube_driver'
require 'active_support/inflector'
require_relative '../animations/animation'
require_relative '../animations/tube/switch_numbers_animation'
require_relative '../animations/tube/single_fly_in_animation'
require_relative 'handler_state_machine'
require_relative '../logging/logging'
require_relative '../messaging/command_receiver'


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
      event :meeting_ticker do
        transition all => :meeting_ticker
      end

      state :startup do
        def write
          NixonPi::Animations::Animation.create(:single_fly_in).run("000123456789")

          if params[:initial_state].nil?
            params[:goto_state] = :time
          else
            params[:goto_state] = params[:initial_state] #todo load initial values
          end

          handle_command(state: params[:goto_state])
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

          handle_command(state: params[:goto_state])
        end
      end

      #todo refactor
      state :countdown do
        require 'chronic_duration'

        def enter_state
          params[:value] = ChronicDuration.parse(params[:value], format: :chrono)
          params[:target_time] = Time.now + params[:value].seconds
        end

        def write
          seconds_to_go = params[:value].to_i
          current_time = Time.now

          if seconds_to_go > 0
            output = ChronicDuration.output(seconds_to_go, :format => :chrono).gsub(/:/, ' ')
            if output != @last_output #only write once
              log.debug "countdown: #{output}"
              @last_output = output
              NixonPi::Messaging::CommandSender.new.send_command(:sound, {value: "#{seconds_to_go}"}) if seconds_to_go <= 10
              driver.write(output)
            end
          end

          params[:value] = params[:target_time] - current_time

          #todo refactor
          if current_time >= params[:target_time]
            log.debug "end of countdown"
            NixonPi::Messaging::CommandSender.new.send_command(:sound, {value: "strike_12.mp3"})
            options = params[:options]
            unless options.nil?
              if options.is_a? Hash
                options.keys.each do |k, o|
                  case k.to_sym
                    when :say
                      NixonPi::Messaging::CommandSender.send_command(:sound, {value: o})
                    when :state
                      handle_command(state: o)
                      return
                    else
                      log.debug "option unknown"
                  end
                end
              end
            end
            handle_command(state: params[:goto_state])
          end
        end
      end

      state :meeting_ticker do
        def enter_state
          #lock lamps
          #find out if initial values or not....
          value = params[:value]

          value.match(/.\s(\d+):(\d+)/)
          if $1 and $2
            @meeting_start = Time.now
            @attendees, @hourly_rate = $1, $2
            @command_sender.send_command(:lamp5, {state: :free_value, locking: :lock, value: 1})
          else
            error_msg = "invalid input for attendees - going to last state"
            NixonPi::Messaging::CommandSender.new.send_command(:sound, {value: error_msg})
            log.error error_msg
            handle_command(state: :time)
          end

        end

        def leave_state
          #unlock lamps
          @command_sender.send_command(:lamp5, {state: :free_value, locking: :unlock, value: 0})
        end

        def write
          per_second_burn = @hourly_rate.to_i * @attendees.to_i / 3600
          elapsed_seconds = Time.now - @meeting_start
          cost = per_second_burn * elapsed_seconds
          driver.write(cost.round(2).to_s.gsub(/\./, " "))
        end
      end
    end
  end
end
