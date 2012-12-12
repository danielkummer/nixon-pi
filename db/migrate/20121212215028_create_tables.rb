class CreateTables < ActiveRecord::Migration
  def up
    create_table :tubes do |t|
      t.string :mode
      t.string :value
      t.string :animation_name
      t.string :options
      t.boolean :initial, default: false
    end
    create_table :bars do |t|
      t.string :mode
      t.string :value
      t.string :animation_name
      t.string :options
      t.boolean :initial, default: false
    end
     create_table :lamps do |t|
      t.string :mode
      t.string :value
      t.string :animation_name
      t.string :options
      t.boolean :initial, default: false
    end

  end

  def down
    drop_table :tubes
    drop_table :bars
    drop_table :lamps
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