class AddSchedulesTable < ActiveRecord::Migration
  def up
     create_table :schedules do |t|
       t.string :timing
       t.string :time
       t.string :state_machine
       t.string :state
       t.string :value
       t.boolean :lock, default: false
     end

   end

   def down
     drop_table :schedules
   end
end
