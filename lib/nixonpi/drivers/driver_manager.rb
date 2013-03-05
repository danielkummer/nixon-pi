require_relative 'driver'
module NixonPi
  class DriverManager


    @@drivers = {}

    class << self
      def register(options = {})
        @@drivers.merge!(options)
      end

      def driver_for(identifier)
        @@drivers[identifier]
      end
    end


  end
end