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
# @param [Hash] args
def get_injected(type, args = {})
  #todo handle multi instances differently - save the instance separately and create it using the blueprint type
  if type.match(/([a-zA-Z]+)\d+$/)
    if @@class_registry.has_key?(type.to_sym)
      reg = @@class_registry[type.to_sym]
      return reg[:instance] if reg.has_key?(:instance)
    end
    if @@class_registry.has_key?($1.to_sym)
      original = @@class_registry[$1.to_sym]
      @@class_registry[type.to_sym] = Marshal.load(Marshal.dump(original))
    else
      raise "no base configuration found for instance type #{type}"
    end
  end

  if @@class_registry.has_key?(type.to_sym)
    reg = @@class_registry[type.to_sym]

    return reg[:instance] if reg.has_key?(:instance)
    options = reg[:args].merge(args)

    if reg[:klass].instance_method(:initialize).parameters.empty?
      klass = reg[:klass].new()
    else
      klass = reg[:klass].new(options)
    end


    klass.send(:registered_as_type=, type) if klass.respond_to?(:registered_as_type=)
    return reg[:instance] = klass
  else
    raise "could not instanciate #{type}"
  end
end

