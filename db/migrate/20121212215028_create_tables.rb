class CreateTables < ActiveRecord::Migration
  def up
    create_table :commands do |t|
      t.string :state_machine
      t.string :state
      t.string :value
      t.string :animation_name
      t.string :options
      t.boolean :initial, default: false
    end

  end

  def down
    drop_table :commands
  end
end

=begin
 DB.create_table :tubes do
    id :primary_key
    text :mode
    text :value
    text :animation_name
    text :options
    boolean :initial, :default => false
  end

  DB.create_table :bars do
    id :primary_key
    text :mode
    text :value
    text :animation_name
    text :options
    boolean :initial, :default => false
  end

  DB.create_table :lamps do
    id :primary_key
    text :mode
    text :value
    text :animation_name
    text :options
    boolean :initial, :default => false
  end

=end