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

  validate :valid_tubes?, :if => "state_machine == :tubes"
  validate :valid_bars?, :if => "state_machine == :bars"
  validate :valid_lamps?, :if => "state_machine == :lamps"
  validate :valid_say?, :if => "state_machine == :say"
  validate :valid_power?, :if => "state_machine == :power"

  def valid_tubes?
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

  def valid_bars?
    case self.state.to_sym
      when :free_value
        bar_count = NixonPi::Settings.in13_pins.size
        errors.add(:value, "number of bar parameters invalid, must be #{bar_count}") unless value.size == bar_count

        value.each_with_index do |val, i|
          errors.add(:value, "bar no. #{i} has invalid value: #{val}") unless (0..255).include?(val.to_i)
        end
      #todo duplicate code - remove
      when :animation
        errors.add(:animation_name, "animation name can't be blank!") if animation_name.blank?
        begin
          JSON.parse(options)
        rescue JSON::ParserError => e
          errors.add(:options, "options invalid json string: #{e.message}")
        end unless options.blank?
    end
  end

  def valid_lamps?
    lamp_count = NixonPi::Settings.in1_pins.size
    errors.add(:value, "number of bar parameters invalid, must be #{lamp_count}") unless value.size == lamp_count

    value.each do |val|
      errors.add(:value, "bars invalid") unless (0..1).include?(val.to_i)
    end
  end

  def valid_say?
    errors.add(:value, "Nothing to say") if value.blank?
  end

end

def valid_power?
  errors.add(:value, "Invalid power value") unless (0..1).include?(value.to_i)
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
        errors.add(:timing, "Timing #{timing} not supported!")
    end
  end
end