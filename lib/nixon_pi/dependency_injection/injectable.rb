module NixonPi
  module DependencyInjection
    module Injectable
      ##
      # Registers a new class to instantiate when requested using get_injected.
      # Note class constructors must support hash arguments if any...
      #
      # @param [Symbol] as_type
      # @param [Object] klass
      # @param [Hash] constuctor_args
      def register(as_type, klass, constuctor_args = {})
        fail "Registry key #{as_type} already taken - please use a different one because all subclass keys are shared" if NixonPi::DependencyInjection::Container.class_registry.key?(as_type.to_sym)
        NixonPi::DependencyInjection::Container.class_registry[as_type.to_sym] = { klass: klass, args: constuctor_args }
      end

      ##
      # Registers an instance to be returned when as_type
      # is requested. This is mostly intended for tests.
      # @param [Symbol] as_type
      # @param [Object] instance
      def register_instance(as_type, instance)
        NixonPi::DependencyInjection::Container.class_registry[as_type.to_sym] = { instance: instance }
      end
    end
  end
end
