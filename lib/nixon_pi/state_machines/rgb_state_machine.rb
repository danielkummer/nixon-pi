require 'state_machine'

module NixonPi
  class RgbStateMachine < BaseStateMachine
    include NixonPi::DependencyInjection

    register :rgb, self
    accepted_commands :state, :value, :animation_name, :options

    def initialize
      super()
      register_driver NixonPi::DependencyInjection::Container.get_injected(:rgb_proxy)
    end

    state_machine do
      state :free_value do
        def write
          value = params[:value]
          if !value.nil? && value != params[:last_value]
            @driver.write(value)
            params[:last_value] = value
          end
        end
      end

      state :startup do
        def write
          params[:value] = 0
          handle_command(state: :free_value, value: 0)
        end
      end
    end
  end
end
