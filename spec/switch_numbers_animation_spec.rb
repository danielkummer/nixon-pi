require_relative 'spec_helper'
require_relative '../lib/nixieberry/animations/switch_numbers_animation'

describe NixieBerry::Animations::SwitchNumbersAnimation do

  before :each do
    @an = NixieBerry::Animations::Animation.create(:switch_numbers, duration: 3)
  end

  it "should return a correct instance" do
    @an.should be_a(NixieBerry::Animations::SwitchNumbersAnimation)
  end

  it "should run the animation" do
    expected = %W{2345 3456 4567}

    @an.run("1234") do |val, iteration|
      val.should eq expected[iteration]
    end
  end
end
