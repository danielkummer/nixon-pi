require 'drb'

#TODO : refactor this so it works with bunny 2 way communication
module NixonPi
  ##
  # Proxy for information requests
  class InformationProxy
    include Logging

    def initialize
      @receivers = {}
    end

    ##
    # Add object which uses the InfoResponder module
    # @param [InfoResponder] receiver_instance
    # @param [Symbol] target name under which the receiver listens for information requests
    def add_target(receiver_instance, target)
      target = target.to_sym
      if @receivers[target].nil?
        @receivers[target] = Array.wrap(receiver_instance)
      else
        @receivers[target] << receiver_instance
      end
    end

    ##
    # Get information from a specified target
    # @param [Symbol] target information from whom
    # @param [Symbol] about information about what
    def get_info_from(target, about)
      target = target.to_sym
      returned_info = []
      if @receivers.key?(target)
        @receivers[target].each do |receiver|
          if receiver.respond_to?(:handle_info_request)
            returned_info << receiver.handle_info_request(about)
          else
            log.error "Listener for #{target} doesn't have the receive method!"
          end
        end
      end
      returned_info = returned_info[0] if returned_info.size == 1 # array to single if only one element
      Marshal.load(Marshal.dump(returned_info))
    end
  end
end
