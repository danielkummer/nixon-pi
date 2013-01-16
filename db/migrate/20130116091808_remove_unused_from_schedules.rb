class RemoveUnusedFromSchedules < ActiveRecord::Migration
  def up
    remove_column :schedules, :state
  end

  def down
    add_column :schedules, :state, :string
  end
end
