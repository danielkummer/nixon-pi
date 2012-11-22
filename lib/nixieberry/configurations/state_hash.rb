require 'singleton'
require_relative 'settings'
#require 'active_support/core_ext/hash/indifferent_access'
require 'json'

module NixieBerry
  class StateHash < ::Hash
    require 'sync'



    def initialize(*args, &block)
      super
    ensure
      extend Sync_m
    end

    def sync(*args, &block)
      sync_synchronize(*args, &block)
    end

    def [](key)
      sync(:SH) { super }
    end

    def []=(key, val)
      sync(:EX) { super }
    end

    # Called for dynamically-defined keys, and also the first key deferenced at the top-level, if load! is not used.
    # Otherwise, create_accessors! (called by new) will have created actual methods for each key.
    def method_missing(name, *args, &block)
      key = name.to_s
      raise("Missing key '#{key}'") unless has_key? key
      value = fetch(key)
      create_accessor_for(key)
      value.is_a?(Hash) ? self.class.new(value) : value
    end

    def create_accessor_for(key, val=nil)
      return unless key.to_s =~ /^\w+$/ # could have "some-setting:" which blows up eval
      instance_variable_set("@#{key}", val) if val
      self.class.class_eval <<-EndEval
        def #{key}
          return @#{key} if @#{key}
          raise ("Missing key '#{key}'") unless has_key? '#{key}'
          value = fetch('#{key}')
          @#{key} = value.is_a?(Hash) ? self.class.new(value) : value
        end
      EndEval
    end
  end

  def to_json
    JSON.encode(self)
  end








end