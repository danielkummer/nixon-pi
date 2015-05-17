module NixonPi
  module DependencyInjection
    def self.included(base)
      base.extend NixonPi::DependencyInjection::Injectable
    end
  end
end


module NixonPi
  module DependencyInjection
    class Container
      @@class_registry = {}


      class << self
        def class_registry
          @@class_registry
        end

        ##
        # Expects the args of a constructor.
        # If an instance  does not exist it instantiates a new instance of type.
        # Alternatively it checks if a dependency has been registered manually and instantiates that.
        #
        # @param [Symbol] type
        # @param [Boolean] new_instance create new instance if true
        # @param [Hash] args new_instance must be true in order to use args
        # TODO: refactor!!!!
        def get_injected(type, new_instance = false, args = {})
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

            return reg_entry[:instance] if reg_entry.key?(:instance) && new_instance == false
            options = reg_entry[:args].merge(args)

            klass = reg_entry[:klass].instance_method(:initialize).parameters.empty? ?
                reg_entry[:klass].new :
                reg_entry[:klass].new(options)

            klass.send(:registered_as_type=, type) if klass.respond_to?(:registered_as_type=)

            new_instance ? klass : reg_entry[:instance] = klass
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
          reg_entry = @@class_registry[type.to_sym]
          reg_entry[:klass]
        end
      end


    end
  end
end