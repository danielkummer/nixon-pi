require 'thread'
require_relative '../logging/logging'
require_relative '../drivers/basic/tube_driver'
require_relative '../factory'
#useage
#Animation.create(:switch_numbers).run

module NixonPi
  module Animations
    class Animation
      include Logging
      include Factory

      attr_accessor :thread

      def initialize()
        super()
        @semaphore = Mutex.new
        @options ||= {}
        @driver = DriverManager.driver_for(:in12a)
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