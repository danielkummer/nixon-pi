module NixonPi
  module CommandReceiver

    ##
    # Process a command received from the command processor
    # @param [Hash] command
    def receive(command)
      raise NotImplementedError
    end

    module ClassMethods

      #basic commands, every receiver has them...
      @@accepted_commands = {}

      def accepted_commands(*names)

        #raise "Command  #{n} already exists - no duplicates allowed!" if @@accepted_commands.include?(n)
        @@accepted_commands[self.name] = names

      end

      def available_commands
        @@accepted_commands[self.name]
      end

    end

    def self.included(base)
      base.extend(ClassMethods)
    end
  end
end