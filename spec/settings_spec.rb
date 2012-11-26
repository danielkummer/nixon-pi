require_relative 'spec_helper'

require_relative '../lib/nixonpi/configurations/settings'

describe NixonPi::Settings do
  before :all do
    @settings = NixonPi::Settings
    @file_name = "nixonpi-settings.yml"
  end

  after :all do
    FileUtils.rm(File.join(Dir.home, @file_name))
  end

  it "should copy the settings file to the users home directory" do
    File.exist?(File.join(Dir.home, @file_name)).should be_true
  end

  it "should successfully save additional configs " do
    @settings['hello'] = 'world'
    @settings.save
    @settings.reload!
    @settings.hello.should eq 'world'
  end
end