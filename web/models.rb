require 'active_record'
require 'active_model/validations'
require 'chronic_duration'
require_relative '../lib/blank_monkeypatch'
require_relative '../lib/nixonpi/configurations/settings'

class Command < ActiveRecord::Base

  attr_accessible :target,
                  :state,
                  :value,
                  :animation_name,
                  :options

  validates_presence_of :target


  #todo refactor!
  validate :valid_tubes?, :if => Proc.new { |c| c.target.to_s.include? "tubes" }
  validate :valid_bar?, :if => Proc.new { |c| c.target.to_s.include? "bar" }
  validate :valid_lamp?, :if => Proc.new { |c| c.target.to_s.include? "lamp" }
  validate :valid_say?, :if => Proc.new { |c| c.target.to_s.include? "say" }
  validate :valid_power?, :if => Proc.new { |c| c.target.to_s.include? "power" }
  validate :valid_rgb?, :if => Proc.new { |c| c.target.to_s.include? "rgb" }

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
        validate_animation
      when :countdown
        unless ChronicDuration.parse(value, format: :chrono)
          errors.add(:value, "countdown format invalid - unable to parse!")
        end
      when :meeting_ticker
        unless value =~ /\d+:\d+/
          errors.add(:value, "enter in the form of attendees:hourly_rate")
        end
      else
        errors.add(:state, "unsupported state")
    end
  end

  def valid_bar?
    case self.state.to_sym
      when :free_value
        errors.add(:value, "#{state_machine} has invalid value: #{value}") unless (0..255).include?(value.to_i)
      #todo duplicate code - remove
      when :animation
        validate_animation
      else
        errors.add(:state, "unsupported state")
    end
  end

  def validate_animation
    errors.add(:animation_name, "animation name can't be blank!") if animation_name.blank?
    errors.add(:animation_name, "options can't be blank!") if options.blank?

    option_hash = HashWithIndifferentAccess.new(options)
    errors.add(:options, "options must include a start_value key") if !option_hash.has_key?(:start_value) or option_hash[:start_value].to_s == ''
    errors.add(:options, "options must include a goto_state key") if !option_hash.has_key?(:goto_state) or option_hash[:goto_state].to_s == ''
    errors.add(:options, "options must include a goto_target key") if !option_hash.has_key?(:goto_target) or option_hash[:goto_target].to_s == ''
  end

  def valid_lamp?
    case self.state.to_sym
      when :free_value
        errors.add(:value, "bars invalid") unless (0..1).include?(value.to_i)
      else
        errors.add(:state, "unsupported state")
    end

  end

  def valid_say?
    errors.add(:value, "Nothing to say") if value.blank?
  end

  def valid_power?
    errors.add(:value, "Invalid power value") unless (0..1).include?(value.to_i)
  end

  def valid_rgb?
    value.gsub!(/^#/, '')
    unless value =~ /^([a-fA-F0-9]{6})$/
      errors.add(:value, "Invalid rgb value")
    end
  end

end


class Schedule < ActiveRecord::Base
  attr_accessible :id,
                  :method,
                  :timing,
                  :target,
                  :command,
                  :lock

  validates_presence_of :target, :method, :timing, :command

  validate :valid_schedule?

  def valid_schedule?
    require 'rufus-scheduler'
    case method.to_sym
      when :in, :every
        begin
          Rufus.parse_time_string(timing)
        rescue ArgumentError => e
          errors.add(:timing, "Schedule invalid for method #{method} with timing #{timing}: #{e.message}")
        end
      when :at
        begin
          t = Time.parse(timing)
          errors.add(:timing, "At timing must be in the future!") if t < Time.now
        rescue ArgumentError => e
          errors.add(:timing, "Schedule invalid for method #{method} with timing #{timing}: #{e.message}")
        end
      when :cron
        #todo validate cron string
      else
        errors.add(:method, "Method #{method} not supported!")
    end unless method.nil?
  end
end