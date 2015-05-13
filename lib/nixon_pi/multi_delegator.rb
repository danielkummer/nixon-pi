module NixonPi

  # Class which can delegate method calls to multiple targets
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

    class << self
      alias_method :to, :new
    end
  end
end
