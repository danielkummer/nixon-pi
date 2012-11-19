require 'thread'
require_relative '../logging/logging'
require_relative '../drivers/tube_driver'
require_relative '../factory'
#useage
#Animation.create(:switch_numbers).run

module NixieBerry
  module Animations
    class Animation
      include Logging
      extend Factory

      attr_accessor :thread

      def initialize
        @semaphore = Mutex.new
        @options ||= {}
        @driver = TubeDriver.instance
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


      def time_diff_milli(start, finish)
         (finish - start) * 1000.0
      end
    end
  end
end