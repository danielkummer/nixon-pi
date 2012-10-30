require 'state_machine'

require_relative '../logging/logging'
require_relative '../configurations/configuration'
require_relative '../drivers/tube_driver'
require_relative '../configurations/control'

module NixieBerry
  class TubeHandlerStateMachine
    include Logging
    include Configuration

    def initialize
      @old_values = {}
      @driver = NixieBerry::TubeDriver.instance
      @controlconfig = NixieBerry::Control.instance

      super() # NOTE: This *must* be called, otherwise states won't get initialized
    end

    state_machine :initial => :display_time do
      around_transition do |object, transition, block|
        object.log.debug "doing transition  #{transition.event} from state: #{object.state}"
        block.call
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

      state :display_time do
        def write_to_tubes
          tubes_count = Settings.in12a_tubes.count
          format = @controlconfig[:time_format]

          if format.nil? or format.size > tubes_count
            log.debug "Using default time format, 6 tubes needed"
            format = Settings.default_time_format
          end

          time = Time.now.strftime(format).rjust(tubes_count, ' ')
          @driver.write(time) unless time == @old_values[:time] or time.nil?
          @old_values[:time] = time
        end
      end

      state :display_free_value do
        def write_to_tubes
          value = @controlconfig[:free_value]
          @driver.write(value) unless value == @old_values[:free_value] or value.nil?
          @old_values[:free_value] = value
        end
      end

      state :display_tube_animation do
        def write_to_tubes
          #todo
          #get animation type
          #start animation thread
          #continue - don' start again if running...
        end
      end


      #todo add animations and transitions module - start animations in threads and use locks to write to correct driver

    end
  end
end