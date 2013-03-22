require_relative 'driver'
module NixonPi
  class DriverManager
    @@drivers = {}

    class << self
      undef_method :new

      def register(options = {})
        options.each_value do |d|
          raise "invalid driver found, driver #{d.class.name} must implement Driver module" unless d.is_a?(Driver)
        end

        @@drivers.merge!(options)
      end

      def instance_for(identifier)
        raise "no driver found for #{identifier.to_s}" if @@drivers[identifier].nil?
        @@drivers[identifier]
      end
    end


  end
end