require_relative 'spec_helper'
require_relative '../lib/nixonpi/animations/tube/single_fly_in_animation'

describe NixonPi::Animations::SingleFlyInAnimation do

  before :each do
    get_injected(:single_fly_in, duration: 3)
  end

  it "should return a correct instance" do
    @an.should be_a(NixonPi::Animations::SingleFlyInAnimation)
  end

  it "should run the animation" do
    expected = %W{4___ _4__ __4_ ___4 3__4 _3_4 __34 2_34 _234 1234}

    @an.run("1234") do |val, iteration|
      val.should eq expected[iteration]
    end
  end
end
