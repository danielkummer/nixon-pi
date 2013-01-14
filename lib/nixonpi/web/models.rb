require 'active_record'
#This is automagically linked with the plural table (todos)
class Command < ActiveRecord::Base
  attr_accessible :state_machine,
                  :state,
                  :value,
                  :animation_name,
                  :options,
                  :initial

  validates_presence_of :state_machine, :state
end

class Schedule < ActiveRecord::Base
  attr_accessible :timing,
                  :time,
                  :state_machine,
                  :state,
                  :value,
                  :lock

  validates_presence_of :state_machine, :state
end