require 'thread'
require_relative '../logging/logging'
require_relative '../drivers/basic/tube_driver'
require_relative '../../dependency'
require_relative '../messaging/command_sender'
require_relative '../messaging/commands_module'
require_relative '../information/information_holder'

module NixonPi
  module Animations
    class Animation
      include Logging
      include Commands
      include InformationHolder

      attr_accessor :thread

      def initialize(options = {})
        raise "options must be a hash" unless options.is_a?(Hash)
        @options = self.class.defaults.merge(options)
        @output = Array.new
      end

      def use_driver(driver)
        @driver = driver
      end

      ##
      # Write to either the driver or a given block
      # leaves the state if no more values to write
      # @param [String] value  value to write
      def handle_output_on_tick(value)
        log.debug "animation value: #{value.to_s}"
        if block_given?
          yield value, index
        else
          if value.nil?
            raise "Options musn't be empty" if @options.nil?
            @send_command ||= begin
              log.debug "Animation ended, sending transition command, target: #{@options[:goto_target]} state: #{@options[:goto_state]}"
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

      def self.handle_info_request(about)
        ret = {}
        case about.to_sym
          when :options
            ret = self.class.defaults
          else
            log.error "No information about #{about}"
        end
        ret
      end

      def self.defaults
        options_hash = Hash.new
        options = available_commands
        options.each do |o|
          default_value =
              case o.to_s
                when /_array$/
                  Array.new
                when /\?$/
                  false
                else
                  nil
              end
          options_hash[o] = default_value
        end
        options_hash
      end
    end
  end
end