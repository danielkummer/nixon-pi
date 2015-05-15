module NixonPi
  module Commands
    module ClassMethods
      @@accepted_commands = {}
      def accepted_commands(*names)
        # raise "Command  #{n} already exists - no duplicates allowed!" if @@accepted_commands.include?(n)
        @@accepted_commands[name] = names
      end

      def available_commands
        @@accepted_commands[name]
      end
    end

    def self.included(base)
      base.extend(ClassMethods)
    end
  end
end
