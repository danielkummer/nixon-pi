module NixonPi
  class DriverManager
    @@drivers = {}
    class << self

      def add_driver(identifier, instance)
        @@drivers[identifier] = instance
      end

      def driver_for(identifier)
        @@drivers[identifier]
      end

    end

  end
end