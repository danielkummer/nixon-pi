require 'state_machine'
require_relative '../drivers/proxies/lamp_proxy'
require_relative 'base_state_machine'
require_relative '../configurations/settings'
require_relative '../drivers/hardware_driver_factory'

module NixonPi
  class RgbStateMachine <  BaseStateMachine

    register_as :rgb
    accepted_commands :state, :value, :animation_name, :options

    def initialize()
      super()
      register_driver HardwareDriverFactory.instance_for(:rgb)
    end

    state_machine do

      state :free_value do
        def write
          value = params[:value]
          if !value.nil? and value != params[:last_value]
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