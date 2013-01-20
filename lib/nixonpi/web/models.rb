require 'active_record'
require 'active_model/validations'
require_relative '../../blank_monkeypatch'
require_relative '../configurations/settings'

class Command < ActiveRecord::Base

  attr_accessible :state_machine,
                  :state,
                  :value,
                  :animation_name,
                  :options,
                  :initial

  validates_presence_of :state_machine

  validate :valid_tubes? #, :valid_bars?, :valid_lamps?, :valid_say?

  def valid_tubes?
    return unless self.state_machine == :tubes
    case self.state.to_sym
      when :free_value
        unless value =~ /\A(\s*\d*)+\s*\Z/
          errors.add(:value, "only numbers and whitespaces are allowed")
        end
        unless value.size <= NixonPi::Settings.in12a_tubes.count
          errors.add(:value, "length exceeds number of tubes (#{NixonPi::Settings.in12a_tubes.count})")
        end
      when :time
        unless value.blank?
          errors.add(:value, "not a valid time format string") unless value =~ /^(\s*%[a-zA-Z]+)+\s*$/
        end
      when :animation
        errors.add(:animation_name, "animation name can't be blank!") if animation_name.blank?
        begin
          JSON.parse(options)
        rescue JSON::ParserError => e
          errors.add(:options, "options invalid json string: #{e.message}")
        end unless options.blank?
      when :countdown
        errors.add(:value, "Countdown format invalid - unable to parse!") if ChronicDuration.parse(value, format: :chrono).nil?
    end
  end


end

class Schedule < ActiveRecord::Base
  attr_accessible :id,
                  :method,
                  :timing,
                  :queue,
                  :command,
                  :lock

  validates_presence_of :queue, :method, :timing, :command

  validate :valid_schedule?

  def valid_schedule?
    require 'rufus-scheduler'
    case timing.to_sym
      when :in, :every
        begin
          Rufus.parse_time_string(time)
        rescue ArgumentError => e
          errors.add(:timing, "Schedule invalid for timing #{timing} with time #{time}: #{e.message}")
        end
      when :at
        begin
          Time.parse(time)
        rescue ArgumentError => e
          errors.add(:time, "Schedule invalid for timing #{timing} with time #{time}: #{e.message}")
        end
      when :cron
        #todo validate cron string
      else
        errors.add(:timing,"Timing #{timing} not supported!")
    end
  end
end