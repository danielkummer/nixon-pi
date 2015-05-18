require 'state_machine'
require 'active_support/inflector'

module NixonPi
  class TubeStateMachine < BaseStateMachine
    include Logging
    include DependencyInjection

    register :tubes, self
    accepted_commands :state, :value, :animation_name, :options, :initial_mode

    def initialize
      super()
      register_driver self.class.get_injected(:in12a_driver)
    end

    state_machine do
      event(:time) { transition all => :time }
      event(:countdown) { transition all => :countdown }
      event(:meeting_ticker) { transition all => :meeting_ticker }

      state :startup do
        def write
          # transition over to the animation state after setting the correct values
          goto_state = params[:initial_state].nil? ? :time : params[:initial_state]
          params[:animation_name] = :single_fly_in
          params[:options] = { start_value: '000123456789', goto_state: goto_state, goto_target: :tubes }
          handle_command(state: :animation)
        end
      end

      state :time do
        def enter_state
          @format = params[:value]
          @tubes_count = Settings.in12a_tubes.count
          @format = nil unless @format.is_a?(String)
          @format = Settings.default_time_format if @format.nil? || @format.size > @tubes_count || @format.strip.empty?

          get_injected(:cmd_send).send_command(:lamp0, state: :free_value, value: 0)
          get_injected(:cmd_send).send_command(:lamp1, state: :free_value, value: 0)
          get_injected(:cmd_send).send_command(:lamp2, state: :free_value, value: 0)
          get_injected(:cmd_send).send_command(:lamp3, state: :free_value, value: 1)
          get_injected(:cmd_send).send_command(:lamp4, state: :blink)
        end

        def write
          now = Time.now
          params[:last_time] = now if params[:last_time].nil?

          formatted_time = now.strftime(@format)
          formatted_date = now.strftime(Settings.default_date_format)

          if now.hour == 12 && now.min == 0 && now.min != params[:last_time].min
            get_injected(:cmd_send).send_command(:sound, value: 'strike_12.mp3') unless @gong_sent
            @gong_sent = true
          else
            @gong_sent = false
          end

          if now.min == 0 && now.min != params[:last_time].min
            params[:animation_name] = :single_fly_in
            params[:options] = { start_value: formatted_time, goto_state: :time, goto_target: :tubes }
            handle_command(state: :animation)
          elsif now.hour == 0 && now.hour != params[:last_time].hour
            params[:animation_name] = :single_fly_in
            params[:options] = { start_value: formatted_time, goto_state: :time, goto_target: :tubes }
            handle_command(state: :animation)
          elsif now.min % 15 == 0 && now.sec <= 10
            @driver.write(formatted_date.rjust(@tubes_count, ' '))
          else
            @driver.write(formatted_time.rjust(@tubes_count, ' '))
          end

          params[:last_value] = formatted_time
          params[:last_time] = now
        end

        def leave_state
          get_injected(:cmd_send).send_command(:lamp3, state: :free_value, value: 0)
          get_injected(:cmd_send).send_command(:lamp4, state: :free_value, value: 0)
        end
      end

      state :free_value do
        def write
          value = params[:value]
          @driver.write(value)
          params[:last_value] = value
        end
      end

      # TODO: refactor
      state :countdown do
        require 'chronic_duration'

        def enter_state
          params[:value] = ChronicDuration.parse(params[:value], format: :chrono)
          params[:target_time] = Time.now + params[:value].seconds
          params[:goto_state] = :time unless %w(time free_value meeting_ticker).include? params[:goto_state].to_s
        end

        def write
          seconds_to_go = params[:value].to_i
          current_time = Time.now

          if seconds_to_go > 0
            output = ChronicDuration.output(seconds_to_go, format: :chrono).gsub(/:/, ' ')

            if output != @last_output # only write once
              log.debug "countdown: #{output}"
              @last_output = output
              get_injected(:cmd_send).send_command(:sound, value: "#{seconds_to_go}") if seconds_to_go <= 10
              @driver.write(output.rjust(@tubes_count, ' '))
            end
          end

          params[:value] = params[:target_time] - current_time

          # TODO: refactor
          if current_time >= params[:target_time]
            log.debug 'end of countdown'
            get_injected(:cmd_send).send_command(:sound, value: 'strike_12.mp3')
            options = params[:options]
            unless options.nil?
              if options.is_a? Hash
                options.keys.each do |k, o|
                  case k.to_sym
                    when :say
                      NixonPi::Messaging::CommandSender.send_command(:sound, value: o)
                    when :state
                      handle_command(state: o)
                      return
                    else
                      log.debug 'option unknown'
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
          # lock lamps
          # find out if initial values or not....
          value = params[:value]

          value.match(/.\s(\d+):(\d+)/)
          if Regexp.last_match(1) && Regexp.last_match(2)
            @meeting_start = Time.now
            @attendees = Regexp.last_match(1)
            @hourly_rate = Regexp.last_match(2)
            get_injected(:cmd_send).send_command(:lamp5, state: :free_value, locking: :lock, value: 1)
          else
            error_msg = 'invalid input for attendees - going to last state'
            get_injected(:cmd_send).send_command(:sound, value: error_msg)
            log.error error_msg
            handle_command(state: :time)
          end
        end

        def leave_state
          # unlock lamps
          get_injected(:cmd_send).send_command(:lamp5, state: :free_value, locking: :unlock, value: 0)
        end

        def write
          per_second_burn = @hourly_rate.to_i * @attendees.to_i / 3600
          elapsed_seconds = Time.now - @meeting_start
          cost = per_second_burn * elapsed_seconds
          @driver.write(sprintf('%.2f', cost).gsub(/\./, ' ').rjust(@tubes_count, ' '))
        end
      end
    end
  end
end
