require 'active_record'
require 'active_model/validations'
require 'chronic_duration'
require_relative 'validators/command_validator'

ActiveRecord::Base.logger = nil

class Command < ActiveRecord::Base
  serialize :options, Hash

  include ActiveModel::Validations
  include ActiveModel::ForbiddenAttributesProtection

  validates_presence_of :target
  validates_with CommandValidator

  #def assign_attributes(values, options = {})
  #  sanitize_for_mass_assignment(values, options[:as]).each do |k, v|
  #    send("#{k}=", v)
  #  end
  #end
end

class Schedule < ActiveRecord::Base
  validates_presence_of :target, :method, :timing, :command

  validate :valid_schedule?

  def valid_schedule?
    require 'rufus-scheduler'
    case method.to_sym
      when :in, :every
        begin
          Rufus::Scheduler.parse(timing)
        rescue ArgumentError => e
          errors.add(:timing, "Schedule invalid for method #{method} with timing #{timing}: #{e.message}")
        end
      when :at
        begin
          t = Time.parse(timing)
          errors.add(:timing, 'At timing must be in the future!') if t < Time.now
        rescue ArgumentError => e
          errors.add(:timing, "Schedule invalid for method #{method} with timing #{timing}: #{e.message}")
        end
      when :cron
        # TODO: validate cron string
      else
        errors.add(:method, "Method #{method} not supported!")
    end unless method.nil?
  end
end
