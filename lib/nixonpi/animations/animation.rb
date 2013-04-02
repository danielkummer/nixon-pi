require 'thread'
require_relative '../logging/logging'
require_relative '../drivers/basic/tube_driver'
require_relative '../../dependency'
require_relative '../messaging/command_sender'

module NixonPi
  module Animations
    class Animation
      include Logging

      attr_accessor :thread

      def use_driver(driver)
        @driver = driver
      end

      def prepare(options = {start_value: ''})
        ;
      end

      ##
      # Write to either the driver or a given block
      # leaves the state if no more values to write
      # @param [String] value  value to write
      def wrapped_write(value)
        log.info "Write animation value: #{value}"
        if block_given?
          yield value, index
        else
          if value.nil?
            raise "Options musn't be empty" if @options.nil?
            @send_command ||= begin
              NixonPi::Messaging::CommandSender.new.send_command(@options[:goto_target], {state: @options[:goto_state]})
              log.debug "already sent transistion command"
            end
          else
            @driver.write(value)
          end

        end

      end


      def time_diff_milli(start, finish)
        (finish - start) * 1000.0
      end
    end
  end
end