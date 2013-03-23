require 'drb'
require_relative 'information_holder'
require_relative '../logging/logging'
require_relative '../hash_monkeypatch'

module NixonPi
  ##
  # Proxy for information requests
  class InformationProxy
    include Logging

    #include DRb::DRbUndumped


    def initialize
      @receivers = Hash.new
    end

    ##
    # Add object which uses the InformationHolder module
    # @param [InformationHolder] receiver
    # @param [Symbol] information_target name under which the receiver listens for information requests
    def add_info_holder(receiver, information_target)
      information_target = information_target.to_sym
      raise "Receiver must include the receiver module" unless receiver.is_a?(InformationHolder)
      if @receivers[information_target].nil?
        @receivers[information_target] = Array.wrap(receiver)
      else
        @receivers[information_target] << receiver
      end

    end

    ##
    # Get information from a specied target
    # @param [Symbol] target information from whom
    # @param [Hash] about information about what
    def get_info_from(target, about)
      target = target.to_sym
      ret = Array.new
      if @receivers.has_key?(target)
        @receivers[target].each do |receiver|
          if receiver.respond_to?(:handle_info_request)
            data = receiver.handle_info_request(about)
            data.delete_if { |k, v| v.nil? }
            ret << data
          else
            log.error "Listener for #{target} doesn't have the receive method!"
          end
        end
      end
      ret = ret[0] if ret.size == 1 #array to single if only one element
      ret
    end
  end
end

