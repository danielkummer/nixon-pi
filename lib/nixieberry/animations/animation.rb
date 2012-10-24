require 'thread'
require_relative '../logging/logging'
require_relative '../drivers/tube_driver'
#useage
#Animation.create(:switch_numbers).run

module NixieBerry
  module Animations
    class Animation
      include Logging

      @@subclasses = {}

      ##
      # Create a new instance of the specified animation type
      # @param [Symbol] type
      # @param [Hash] options
      def self.create(type, options)
        c = @@subclasses[type]
        if c
          c.new(options)
        else
          raise "Bad type: #{type}"
        end
      end

      ##
      # Register the animation for calling it later with create
      # @param [Symbol] name
      def self.register_animation(name)
        @@subclasses[name] = self
      end

      def initialize
        @semaphore = Mutex.new
        @driver = TubeDriver.instance
        @options ||= {}
      end

      ##
      # Method stub for animations, implement in subclasses
      def run(*)
        raise NotImplementedError
      end

      ##
      # Thread save write to either the driver or a given block
      #
      # @param [String] value  value to write
      # @param [Integer] index iteration index
      def write(value, index)
        @semaphore.synchronize {
          if block_given?
            yield value, index
          else
            @driver.write(value)
          end
        }
      end
    end
  end
end