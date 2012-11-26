module NixonPi
  class MultiDelegator
    def initialize(*targets)
      @targets = targets
    end

    ##
    # Delegate method call to registered targets
    # @param [Method] methods
    def self.delegate(*methods)
      methods.each do |m|
        define_method(m) do |*args|
          @targets.map { |t| t.send(m, *args) }
        end
      end
      self
    end

    class <<self
      alias to new
    end
  end
end