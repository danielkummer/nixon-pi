require_relative 'driver'
module NixonPi
  class DriverManager
    @@drivers = {}

    class << self
      undef_method :new

      def register(options = {})
        raise "invalid driver found, driver must implement "
        @@drivers.merge!(options)
      end

      def instance_for(identifier)
        raise "no driver found for #{identifier.to_s}" unless @@drivers[identifier].nil?
        @@drivers[identifier]
      end
    end


  end
end