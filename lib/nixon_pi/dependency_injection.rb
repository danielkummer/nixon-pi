module NixonPi
  module DependencyInjection
    def self.included(base)
      base.send :include, ClassMethods
      base.extend ClassMethods
    end

    module ClassMethods
      @@class_registry = ThreadSafe::Cache.new

      ##
      # Registers a new class to instantiate when requested using get_injected.
      # Note class constructors must support hash arguments if any...
      #
      # @param [Symbol] as_type
      # @param [Object] klass
      # @param [Hash] constuctor_args
      def register(as_type, klass, constructor_args = {})
        fail "Registry key #{as_type} already taken!" if @@class_registry.key?(as_type.to_sym)
        @@class_registry[as_type.to_sym] = {klass: klass, args: constructor_args}
      end

      ##
      # Expects the args of a constructor.
      # If an instance  does not exist it instantiates a new instance of type.
      # Alternatively it checks if a dependency has been registered manually and instantiates that.
      #
      # @param [Symbol] type
      # @param [Boolean] is_new_instance create new instance if true
      # @param [Hash] args is_new_instance must be true in order to use args
      def get_injected(type, is_new_instance = false, args = {})
        # TODO: handle multi instances differently - save the instance separately and create it using the blueprint type

        if type.match(/([a-zA-Z]+)\d+$/)
          if @@class_registry.key?(type.to_sym)
            reg_entry = @@class_registry[type.to_sym]
            return reg_entry[:instance] if reg_entry.key?(:instance)
          end
          if @@class_registry.key?(Regexp.last_match(1).to_sym) # create new instance if based on a sequential value e.g bar1
            original = @@class_registry[Regexp.last_match(1).to_sym]
            begin
              @@class_registry[type.to_sym] = Marshal.load(Marshal.dump(original))
            rescue Exception => e
              raise("error on #{original}: #{e.message}")
            end
          else
            fail "no base configuration found for instance type #{type}"
          end
        end

        if @@class_registry.key?(type.to_sym)
          reg_entry = @@class_registry[type.to_sym]

          return reg_entry[:instance] if reg_entry.key?(:instance) && is_new_instance == false
          options = reg_entry[:args].merge(args)

          klass = if reg_entry[:klass].instance_method(:initialize).parameters.empty?
                    reg_entry[:klass].new
                  else
                    reg_entry[:klass].new(options)
                  end

          klass.send(:registered_as_type=, type) if klass.respond_to?(:registered_as_type=)

          is_new_instance ? klass : reg_entry[:instance] = klass
        else
          fail "could not instantiate #{type}"
        end

      end

      ##
      # Get a class defined by its type.
      #
      # @param type [Symbol] the class type
      # @return [Object] class
      def get_class(type)
        @@class_registry[type.to_sym][:klass]
      end
    end
  end
end


