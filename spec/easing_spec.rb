require_relative 'spec_helper'
require_relative '../lib/nixieberry/animations/easing'

describe NixieBerry::Easing do

  before :each do
    @object = Object.new
    @object.extend(NixieBerry::Easing)
  end


    # @param [Object] x percent complete (0.0 - 1.0)
    # @param [Object] t elapsed time ms
    # @param [Object] b start value
    # @param [Object] c end value
    # @param [Object] d total duration in ms
  it "should provide quadratic easing" do
    1000.times.with_index do |x|
      # percent complete - val - start - end - max
      puts @object.ease_in_out_quad( x.to_f, 0.0, 255.0, 1000.0)
    end
  end

end
