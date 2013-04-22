@@class_registry = Hash.new


class DependencyNotFound < StandardError;
end


##
# Registers a new class to instantiate when requested using get_injected.
# Note class constructors must support hash arguments if any...
#
# @param [Symbol] as_type
# @param [Object] klass
# @param [Hash] constuctor_args
def register(as_type, klass, constuctor_args = {})
  raise "Registry key #{as_type} already taken - please use another one because all subclass keys are shared" if @@class_registry.has_key?(as_type.to_sym)
  @@class_registry[as_type.to_sym] = {klass: klass, args: constuctor_args}
end

##
# Registers an instance to be returned when as_type
# is requested. This is mostly intended for tests.
# @param [Symbol] as_type
# @param [Object] instance
def register_instance(as_type, instance)
  @@class_registry[as_type.to_sym] = {instance: instance}
end


##
# Expects the args of a constructor.
# If an instance  does not exist it instantiates a new instance of type.
# Alternatively it checks if a dependency has been registered manually and instantiates that.
#
# @param [Symbol] type
# @param [Boolean] new_instance create new instance if true
# @param [Hash] args new_instance must be true in order to use args
# todo refactor!!!!
def get_injected(type, new_instance = false, args = {})
  #todo handle multi instances differently - save the instance separately and create it using the blueprint type
  if type.match(/([a-zA-Z]+)\d+$/)
    if @@class_registry.has_key?(type.to_sym)
      reg_entry = @@class_registry[type.to_sym]
      return reg_entry[:instance] if reg_entry.has_key?(:instance)
    end
    if @@class_registry.has_key?($1.to_sym) #create new instance if based on a sequential value e.g bar1
      original = @@class_registry[$1.to_sym]
      @@class_registry[type.to_sym] = Marshal.load(Marshal.dump(original))
    else
      raise "no base configuration found for instance type #{type}"
    end
  end

  if @@class_registry.has_key?(type.to_sym)
    reg_entry = @@class_registry[type.to_sym]

    return reg_entry[:instance] if reg_entry.has_key?(:instance) and new_instance == false
    options = reg_entry[:args].merge(args)

    klass = reg_entry[:klass].instance_method(:initialize).parameters.empty? ?
        reg_entry[:klass].new() :
        reg_entry[:klass].new(options)

    klass.send(:registered_as_type=, type) if klass.respond_to?(:registered_as_type=)

    return new_instance ? klass : reg_entry[:instance] = klass
  else
    raise "could not instanciate #{type}"
  end
end

def get_class(type)
  reg_entry = @@class_registry[type.to_sym]
  reg_entry[:klass]
end

