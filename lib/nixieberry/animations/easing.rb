module NixieBerry
  module Easing

    ##
    # In-Out Quardratic easing
    # easeOutQuad(val/max, val, start, end, max));
    # @param [Object] x percent complete (0.0 - 1.0)
    # @param [Object] t elapsed time ms
    # @param [Object] b start value
    # @param [Object] c end value
    # @param [Object] d total duration in ms
    def easeInOutQuad(x, t, b, c, d)
      return c/2*t*t + b if (( t /= d/2) < 1)
      -c/2 * ((--t)*(t-2) - 1) + b
    end
  end
end