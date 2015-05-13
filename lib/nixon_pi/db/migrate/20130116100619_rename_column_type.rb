class RenameColumnType < ActiveRecord::Migration
  def up
    rename_column :schedules, :type, :method
  end

  def down
    rename_column :schedules, :method, :type
  end
end
