class RenameScheduleAttributes < ActiveRecord::Migration
  def up
    rename_column :schedules, :timing, :type
    rename_column :schedules, :state_machine, :queue
    rename_column :schedules, :value, :command
  end

  def down
    rename_column :schedules, :type, :timing
    rename_column :schedules, :queue, :state_machine
    rename_column :schedules, :command, :value
  end
end
