module NixonPi

  ##
  # Commands module, allows defining accepted commands for an object on class level.
  # Commands are used for interacting with state machines and proxies.
  #
  module Commands
    def self.included(base)
      base.extend(ClassMethods)
    end

    module ClassMethods
      @@accepted_commands = {}

      ##
      # Define accepted commands which will be checked before a command is routed to the object
      #
      # @param [Array] List of names (symbols) for accepted commands
      def accepted_commands(*names)
        # raise "Command  #{n} already exists - no duplicates allowed!" if @@accepted_commands.include?(n)
        @@accepted_commands[name] = names
      end

      ##
      # Get all available commands for the current object
      #
      # @return [Array] command symbols
      def available_commands
        @@accepted_commands[name]
      end
    end
  end
end
