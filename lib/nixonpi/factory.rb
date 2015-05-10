require_relative 'logging/logging'

module NixonPi
  module Factory
    module ClassMethods
      include Logging
      @@subclasses = {}
      ##
      # Create a new instance of the specified animation type, runs an after_create hook method if defined
      # @param [Symbol] type, if type contains a suffix integer, return the base type class instance
      # @param [Object] options optional hash to be passed to the class instance
      def create(type, options = nil)
        if type.match(/([a-zA-Z]+)\d+/)
          # remove the int suffix to locate the subclass
          klass = @@subclasses[Regexp.last_match(1).to_sym]
        else
          klass = @@subclasses[type.to_sym]
        end

        begin
          if klass
            # if klass.instance_method(:initialize).parameters.empty? or
            if options.nil?
              instance = klass.new
            else
              instance = klass.new(options)
            end
            instance.send(:registered_as_type=, type) if instance.respond_to?(:registered_as_type=)
            instance
          else
            fail "Bad type: #{type}"
          end
        rescue Exception
          log.error "Error in DI: #{$ERROR_INFO.message}"
        ensure
          fail 'Object instanciation error'
        end
      end

      ##
      # Register the object for calling it later with create
      # @param [*Symbol] names one ore more names under which to register the class for the factory methods
      def register_as(*names)
        names.each do |n|
          fail 'Subclass key already taken - please use another one because all subclass keys are shared' if @@subclasses.key?(n)
          @@subclasses[n] = self
          log.debug "registered instance for #{self.class.name} under #{n}"
        end
      end
    end

    def self.included(base)
      base.extend(ClassMethods)
    end
  end
end
