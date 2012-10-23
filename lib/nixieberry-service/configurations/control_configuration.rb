require 'redis'
require 'singleton'
require 'active_support/core_ext/hash/indifferent_access'

##
# Holds the configuration values retrieved from the redis service
# call-seq:
#   ControlConfiguration.instance.update_from_redis
#   ControlConfiguration.instance[:mode]
#
module NixieBerry
  class ControlConfiguration < HashWithIndifferentAccess
    include Singleton
    include NixieConfig

    def initialize
      super()
      redis_config = config.has_key?[:redis] ? config[:redis] : {}
      @redis = Redis.new(redis_config)
    end

    # Get the accepted control keys for controlling the service via redis
    # @return [Array]
    def accepted_keys
      [:mode, :free_value, :bars]
    end


    # Update the control configuration with the current redis values (if set)
    def update_from_redis
      accepted_keys.each do |key|
        value = @redis.get(key.to_s)
        self[key] = value unless value.nil?
      end
    end
  end
end