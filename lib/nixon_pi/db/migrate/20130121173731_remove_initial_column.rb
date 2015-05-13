class RemoveInitialColumn < ActiveRecord::Migration
  def up
    remove_column :commands, :initial
  end

  def down
    add_column :commands, :initial
  end
end
