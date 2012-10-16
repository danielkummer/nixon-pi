require_relative 'nixieberry-service/version'

module NixieBerry

  autoload :NixieConfig, 'nixieberry-service/configurations/nixie_config'
  autoload :NixieLogger, 'nixieberry-service/logging/nixie_logger'
  autoload :MultiDelegator, 'nixieberry-service/delegators/multi_delegator'
  autoload :ControlConfiguration, 'nixieberry-service/configurations/control_configuration'
  autoload :AbioCardClient, 'nixieberry-service/client/abio_card_client'
  autoload :TubeDriver, 'nixieberry-service/drivers/tube_driver'
  autoload :NeonDriver, 'nixieberry-service/drivers/neon_driver'
  autoload :BarDriver, 'nixieberry-service/drivers/bar_driver'
  autoload :BarHandler, 'nixieberry-service/handlers/bar_handler'
  autoload :TubeHandlerStateMachine, 'nixieberry-service/handlers/tube_handler_state_machine'
  autoload :Animate, 'nixieberry-service/animations/animate'
  autoload :AnimationQueue, 'nixieberry-service/animation_queue'

  ## Exceptions
  class RetryError < StandardError;
  end

end
