# http://api.rubyonrails.org/classes/ActiveModel/Validator.html
require_relative '../../lib/blank_monkeypatch'

class CommandValidator < ActiveModel::Validator
  def validate(record)
    case record.target
      when /tubes/
        validate_tubes(record)
      when /bar/
        validate_bars(record)
      when /lamp/
        case record.state
          when /free_value/
            record.errors.add(:value, 'lamp value invalid') unless (0..1).include?(record.value.to_i)
          when /blink/
            # no errors
          else
            record.errors.add(:state, 'unsupported state')
        end
      when /sound/
        record.errors.add(:value, 'Nothing to say') if record.value.blank?
      when /power/
        record.errors.add(:value, 'Invalid power value') unless (0..1).include?(record.value.to_i)
      when /rgb/
        record.errors.add(:value, 'Invalid rgb value') unless record.value.gsub(/^#/, '') =~ /^([a-fA-F0-9]{6})$/
      when /background/
        record.errors.add(:value, "background has invalid value: #{record.value}") unless (0..255).include?(record.value.to_i)
      else
        record.errors.add :target, 'Unknown target'
    end
  end

  private

  def validate_tubes(record)
    case record.state
      when /free_value/
        record.errors.add(:value, 'only numbers and whitespaces are allowed') unless record.value =~ /\A(\s*\d*)+\s*\Z/
        record.errors.add(:value, "length exceeds number of tubes (#{NixonPi::Settings.in12a_tubes.count})") unless record.value.size <= NixonPi::Settings.in12a_tubes.count
      when /time/
        record.errors.add(:value, 'not a valid time format string') unless record.value =~ /^(\s*%[a-zA-Z]+)+\s*$/ unless record.value.blank?
      when /animation/
        validate_animation(record)
      when /countdown/
        record.errors.add(:value, 'countdown format invalid - unable to parse!') unless ChronicDuration.parse(record.value, format: :chrono)
      when /meeting_ticker/
        record.errors.add(:value, 'enter in the form of attendees:hourly_rate') unless record.value =~ /\d+:\d+/
      else
        record.errors.add(:state, 'unsupported state')
    end
  end

  def validate_bars(record)
    case record.state.to_s
      when /free_value/
        record.errors.add(:value, "#{record.target} has invalid value: #{record.value}") unless (0..255).include?(record.value.to_i)
      # TODO: duplicate code - remove
      when /animation/
        validate_animation(record)
      else
        record.errors.add(:state, 'unsupported state')
    end
  end

  def validate_animation(record)
    record.errors.add(:animation_name, "animation name can't be blank!") if record.animation_name.blank?
    record.errors.add(:options, "options can't be blank") if record.options.blank?
    record.errors.add(:options, 'options must be a hash') unless record.options.is_a?(Hash)

    option_hash = HashWithIndifferentAccess.new(record.options)
    record.errors.add(:options, 'options must include a start_value key') if !option_hash.key?(:start_value) || option_hash[:start_value].to_s.blank?
    record.errors.add(:options, 'options must include a goto_state key') if !option_hash.key?(:goto_state) || option_hash[:goto_state].to_s.blank?
    record.errors.add(:options, 'options must include a goto_target key') if !option_hash.key?(:goto_target) || option_hash[:goto_target].to_s.blank?
  end
end
