require 'state_machine'

module NixieBerry
  class TubeHandlerStateMachine
    include NixieLogger
    include NixieConfig

    def initialize
      @old_values = {}
      @driver = NixieBerry::TubeDriver.instance
      @controlconfig = NixieBerry::ControlConfiguration.instance

      super() # NOTE: This *must* be called, otherwise states won't get initialized
    end

    state_machine :initial => :display_time do
      around_transition do |object, transition, block|
        object.log.debug "doing transition  #{transition.event} from state: #{object.state}"
        block.call
        object.log.debug "new state: #{object.state}"
      end

      event :display_free_value do
        transition :display_time => :display_free_value
      end

      event :display_time do
        transition :display_free_value => :display_time
      end

      state :display_time do
        def write_to_tubes
          tubes_count = config[:tubes][:no_of_tubes]
          format = @controlconfig[:time_format]

          if format.nil? or format.size > tubes_count
            log.debug "Using default time format, 6 tubes needed"
            format = config[:default_time_format]
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


      #todo add animations and transitions module - start animations in threads and use locks to write to correct driver

    end
  end
end