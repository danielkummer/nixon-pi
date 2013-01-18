class AddTimingToSchedule < ActiveRecord::Migration
  def up
    add_column :schedules, :timing, :string
  end

  def down
    remove_column :schedules, :timing
  end
end
