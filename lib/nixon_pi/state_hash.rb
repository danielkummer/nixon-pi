require 'singleton'
require 'json'

module NixonPi
  class StateHash < ::Hash
    require 'sync'

    def initialize(*args, &block)
      super
    ensure
      extend Sync_m
    end

    ##
    # Synchronize hash
    def sync(*args, &block)
      sync_synchronize(*args, &block)
    end

    ##
    # Synchronized hash get
    #
    # @param key [Symbol] hash key
    # @return [Object] hash value
    def [](key)
      sync(:SH) { super }
    end

    ##
    # Synchronized hash set
    # @param key [Symbol] hash key
    # @param val [Object] hash value
    def []=(key, val)
      sync(:EX) { super }
    end

    ##
    # Called for dynamically-defined keys, and also the first key deferenced at the top-level, if load! is not used.
    # Otherwise, create_accessors! (called by new) will have created actual methods for each key.
    #
    # @param name [Symbol] method name
    # @param *_args [Object] method arguments
    def method_missing(name, *_args, &_block)
      key = name.to_s
      fail("Missing key '#{key}'") unless key? key
      value = fetch(key)
      create_accessor_for(key)
      value.is_a?(Hash) ? self.class.new(value) : value
    end

    ##
    # Create accessor for key and value
    #
    # @param key [Symbol] accessor name
    # @param val [Object] value
    def create_accessor_for(key, val = nil)
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
