require 'thread'
require 'active_support/hash_with_indifferent_access'

module NixonPi
  module Animations
    class Animation
      include Logging
      extend Logging
      include Commands
      include InfoResponder
      include NixonPi::DependencyInjection

      attr_accessor :thread

      def initialize(options = {})
        fail 'options must be a hash' unless options.is_a?(Hash)
        @options = HashWithIndifferentAccess.new(self.class.defaults.merge(options))
        @output = []
      end

      def use_driver(driver)
        @driver = driver
      end

      ##
      # Write to either the driver or a given block
      # leaves the state if no more values to write
      # @param [String] value  value to write
      def handle_output_on_tick(value)
        if block_given?
          yield value, index
        else
          if value.nil?
            fail "Options musn't be empty" if @options.nil?
            @send_command ||= begin
              log.debug "Animation ended, sending transition command, target: #{@options[:goto_target]} state: #{@options[:goto_state]}"
              get_injected(:cmd_send).send_command(@options[:goto_target], state: @options[:goto_state])
            end
          else
            log.debug "animation value: #{value}"
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
          when :params # animation options, but called 'params' for consistency
            ret = defaults
          else
            log.error "No information about #{about}"
        end
        ret
      end

      def self.defaults
        options_hash = {}
        options = available_commands
        options.each do |o|
          default_value =
              case o.to_s
                when /_array$/
                  []
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
