require 'active_record'
#This is automagically linked with the plural table (todos)
class Command < ActiveRecord::Base
  attr_accessible :state_machine,
                  :state,
                  :value,
                  :animation_name,
                  :options,
                  :initial
end
