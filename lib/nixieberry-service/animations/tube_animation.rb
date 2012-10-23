require 'thread'

#useage
#TubeAnimation.create(:switch_numbers).run

module NixieBerry
  class TubeAnimation
    include AnimationQueue
    include NixieLogger

    @@subclasses = {}

    def self.create(type, options)
      @options = {:sleep => 0.5}.merge(options)

      c = @@subclasses[type]
      if c
        c.new(options)
      else
        raise "Bad type: #{type}"
      end
    end

    def self.register_animation name
      @@subclasses[name] = self
    end

    def initialize
      @driver = AbioCardClient.instance
    end

    def run
      raise NotImplementedError
    end
  end
end