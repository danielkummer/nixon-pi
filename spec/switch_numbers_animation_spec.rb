require_relative 'spec_helper'
require_relative '../lib/nixonpi/animations/tube/switch_numbers_animation'

describe NixonPi::Animations::SwitchNumbersAnimation do
  before :each do
    @an = get_injected(:switch_numbers, duration: 3)
  end

  it 'should return a correct instance' do
    @an.should be_a(NixonPi::Animations::SwitchNumbersAnimation)
  end

  it 'should run the animation' do
    expected = %w(2345 3456 4567)

    @an.run('1234') do |val, iteration|
      val.should eq expected[iteration]
    end
  end
end
