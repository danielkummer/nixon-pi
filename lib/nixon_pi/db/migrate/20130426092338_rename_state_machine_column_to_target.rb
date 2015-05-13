class RenameStateMachineColumnToTarget < ActiveRecord::Migration
  def up
    rename_column :schedules, :queue, :target
    rename_column :commands, :state_machine, :target
  end

  def down
    rename_column :schedules, :target, :queue
    rename_column :commands, :target, :state_machine
  end
end
