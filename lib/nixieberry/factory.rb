module NixieBerry
  module Factory

    module ClassMethods
      @@subclasses = {}
      ##
      # Create a new instance of the specified animation type, runs an after_create hook method if defined
      # @param [Symbol] type
      # @param [Hash] options
      def create(type, options = {})
        klass = @@subclasses[type]
        if klass
          if klass.instance_method(:initialize).parameters.empty? or options.empty?
            instance = klass.new
          else
            instance = klass.new(options)
          end
          instance.send(:registered_as_type=, type)
          instance.send(:after_create) if instance.respond_to?(:after_create)
          instance
        else
          raise "Bad type: #{type}"
        end
      end

      ##
      # Register the object for calling it later with create
      # @param [Symbol] name
      def register_as(name)
        raise "Subclass key already taken - please use another one because all subclass keys are shared" if @@subclasses.has_key?(name)
        @@subclasses[name] = self
      end
    end

    ##
    # Get the symbol under which the class was registered
    attr_accessor :registered_as_type

    def self.included(base)
      base.extend(ClassMethods)
    end

  end
end