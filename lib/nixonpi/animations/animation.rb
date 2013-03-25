require 'thread'
require_relative '../logging/logging'
require_relative '../drivers/basic/tube_driver'
require_relative '../../dependency'

module NixonPi
  module Animations
    class Animation
      include Logging

      attr_accessor :thread

      def initialize()
        super()
        @semaphore = Mutex.new
        @options ||= {}
        @driver = get_injected(:in12a_driver)
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
      # Todo the semaphore should be obsolete now...
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