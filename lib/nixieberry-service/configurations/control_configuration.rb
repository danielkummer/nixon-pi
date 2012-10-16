require 'redis'
require 'singleton'
require 'active_support/core_ext/hash/indifferent_access'

module NixieBerry
  class ControlConfiguration < HashWithIndifferentAccess
    include Singleton
    def initialize
      super()
      @redis = Redis.new
    end

    def accepted_keys
      [:mode, :free_value, :bars]
    end

    def update_from_redis
      accepted_keys.each do |key|
        value = @redis.get(key.to_s)
        self[key] = value unless value.nil?
      end
    end

  end
end