require 'thread'
require 'active_record'

require 'nixon_pi/cli/application'


require 'nixon_pi/dependency_injection/container'
require 'nixon_pi/dependency_injection/injectable'

require 'nixon_pi/state_hash'


require 'nixon_pi/os_info'


require 'nixon_pi/blank_monkeypatch'
require 'nixon_pi/hash_monkeypatch'

require 'nixon_pi/version'
require 'nixon_pi/multi_delegator'
require 'nixon_pi/logging'

require 'nixon_pi/settings'

require 'nixon_pi/retryable'
require 'nixon_pi/hex_bit_convert'

require 'nixon_pi/direct_io'
require 'nixon_pi/mock_telnet'
require 'nixon_pi/information_proxy'
require 'nixon_pi/information_holder'
require 'nixon_pi/hardware_info'
require 'nixon_pi/network_info'
require 'nixon_pi/messaging/client'
require 'nixon_pi/commands'
require 'nixon_pi/messaging/command_receiver'
require 'nixon_pi/messaging/command_sender'

require 'nixon_pi/scheduler/command_job'
require 'nixon_pi/scheduler'

require 'nixon_pi/abio_card_client'

require 'nixon_pi/driver/driver'
require 'nixon_pi/driver/io_driver'
require 'nixon_pi/driver/pwm_driver'
require 'nixon_pi/driver/tube_driver'


require 'nixon_pi/driver/proxy/background_proxy'
require 'nixon_pi/driver/proxy/lamp_proxy'
require 'nixon_pi/driver/proxy/power_proxy'
require 'nixon_pi/driver/proxy/rgb_proxy'
require 'nixon_pi/driver/proxy/sound_proxy'

require 'nixon_pi/animations/animation'
require 'nixon_pi/animations/easing'
require 'nixon_pi/animations/bar/ramp_up_down_animation'
require 'nixon_pi/animations/lamp/blink_animation'
require 'nixon_pi/animations/led/rgb_animation'
require 'nixon_pi/animations/tube/count_up_all_animation'
require 'nixon_pi/animations/tube/single_fly_in_animation'
require 'nixon_pi/animations/tube/switch_numbers_animation'

require 'nixon_pi/state_machines/base_state_machine'
require 'nixon_pi/state_machines/bar_state_machine'
require 'nixon_pi/state_machines/lamp_state_machine'
require 'nixon_pi/state_machines/multi_machine_proxy'
require 'nixon_pi/state_machines/rgb_state_machine'
require 'nixon_pi/state_machines/tube_state_machine'
require 'nixon_pi/state_machines/machine_manager'

require 'nixon_pi/db/models'

require 'nixon_pi/nixie_service'

require 'nixon_pi/web/web_server'


Thread.abort_on_exception = true

module NixonPi
    # nothing to do here...
end