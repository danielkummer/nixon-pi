require 'state_machine'

require_relative '../drivers/basic/tube_driver'
require 'active_support/inflector'
require_relative '../animations/animation'
require_relative '../animations/tube/switch_numbers_animation'
require_relative '../animations/tube/single_fly_in_animation'
require_relative 'base_state_machine'
require_relative '../logging/logging'
require_relative '../messaging/command_receiver'
require_relative '../../dependency'


module NixonPi
  class TubeStateMachine < BaseStateMachine
    include Logging

    register :tubes, self
    accepted_commands :state, :value, :animation_name, :options, :initial_mode

    def initialize()
      super()
      register_driver get_injected(:in12a_driver)
    end

    state_machine do

      event(:time) { transition all => :time }
      event(:countdown) { transition all => :countdown }
      event(:meeting_ticker) { transition all => :meeting_ticker }

      state :startup do
        def write
          #transition over to the animation state after setting the correct values
          goto_state = params[:initial_state].nil? ? :time : params[:initial_state]
          params[:animation_name] = :single_fly_in
          params[:options] = {start_value: '000123456789', goto_state: goto_state, goto_target: :tubes}
          handle_command(state: :animation)
        end

      end

      state :time do
        def enter_state
          @format = params[:value]
          @tubes_count = Settings.in12a_tubes.count
          @format = nil unless @format.is_a?(String)
          @format = Settings.default_time_format if @format.nil? or @format.size > @tubes_count
        end


        def write
          now = Time.now
          params[:last_time] = now if params[:last_time].nil?

          formatted_time = now.strftime(@format)
          formatted_date = now.strftime(Settings.default_date_format)


          if now.min == 0 and now.min != params[:last_time].min
            get_injected(:single_fly_in).run(formatted_time)
          elsif now.hour == 0 and now.hour != params[:last_time].hour
            get_injected(:single_fly_in).run(formatted_time)
          elsif now.min % 15 == 0 and now.sec <= 10
            @driver.write(formatted_date.rjust(@tubes_count, ' '))
          else
            @driver.write(formatted_time.rjust(@tubes_count, ' '))
          end

          params[:last_value] = formatted_time
          params[:last_time] = now

        end
      end

      state :free_value do
        def write
          value = params[:value]
          @driver.write(value)
          params[:last_value] = value
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
              @driver.write(output)
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
          @driver.write(cost.round(2).to_s.gsub(/\./, " "))
        end
      end
    end
  end
end
