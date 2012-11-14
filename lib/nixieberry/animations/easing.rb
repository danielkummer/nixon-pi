module NixieBerry
  ##
  # Provide easing functions, all methods have the same parameters
  # @param [Object] t elapsed time ms
  # @param [Object] b start value
  # @param [Object] c end value
  # @param [Object] d total duration in ms

  module Easing
    # simple linear tweening - no easing, no acceleration
    def tween_linear(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      return c*t/d + b
    end


    # quadratic easing in - accelerating from zero velocity
    def ease_in_quad(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      t /= d
      return c*t*t + b
    end

    # quadratic easing out - decelerating to zero velocity
    def ease_out_quad(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      t /= d
      return -c * t*(t-2) + b
    end

    # quadratic easing in/out - acceleration until halfway, then deceleration
    def ease_in_out_quad(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f

      t /= d/2
      return c/2*t*t + b if (t < 1)
      t -= 1
      return -c/2 * (t*(t-2) - 1) + b
    end

    # cubic easing in - accelerating from zero velocity
    def ease_in_cubic(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      t /= d
      return c*t*t*t + b
    end

    # cubic easing out - decelerating to zero velocity
    def ease_out_cubic(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      t /= d
      t -= 1
      return c*(t*t*t + 1) + b
    end

    # cubic easing in/out - acceleration until halfway, then deceleration
    def ease_in_out_cubic(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      t /= d/2
      return c/2*t*t*t + b if (t < 1)
      t -= 2
      return c/2*(t*t*t + 2) + b
    end

    # quartic easing in - accelerating from zero velocity
    def ease_in_quart(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      t /= d
      return c*t*t*t*t + b
    end

    # quartic easing out - decelerating to zero velocity
    def ease_out_quart(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      t /= d
      t-= 1
      return -c * (t*t*t*t - 1) + b;
    end

    # quartic easing in/out - acceleration until halfway, then deceleration
    def ease_in_out_quart(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      t /= d/2
      return c/2*t*t*t*t + b if (t < 1)
      t -= 2
      return -c/2 * (t*t*t*t - 2) + b
    end

    # quintic easing in - accelerating from zero velocity
    def ease_in_quint(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      t /= d
      return c*t*t*t*t*t + b
    end

    # quintic easing out - decelerating to zero velocity
    def ease_out_quint(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      t /= d
      t-= 1
      return c*(t*t*t*t*t + 1) + b
    end

    # quintic easing in/out - acceleration until halfway, then deceleration
    def ease_in_out_quint(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      t /= d/2
      return c/2*t*t*t*t*t + b if (t < 1)
      t -= 2
      return c/2*(t*t*t*t*t + 2) + b
    end

    # sinusoidal easing in - accelerating from zero velocity
    def ease_in_sine(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      return -c * Math.cos(t/d * (Math.PI/2)) + c + b
    end

    # sinusoidal easing out - decelerating to zero velocity
    def ease_out_sine(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      return c * Math.sin(t/d * (Math.PI/2)) + b
    end

    # sinusoidal easing in/out - accelerating until halfway, then decelerating
    def ease_in_out_sine(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      return -c/2 * (Math.cos(Math.PI*t/d) - 1) + b
    end

    # exponential easing in - accelerating from zero velocity
    def ease_in_expo(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      return c * 2 ** (10 * (t/d - 1)) + b
    end

    def ease_out_expo(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      return c * (-2 ** (-10 * t/d) + 1) + b
    end

    # exponential easing in/out - accelerating until halfway, then decelerating
    def ease_in_out_expo(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      t /= d/2
      return c/2 * 2 ** 10 * (t - 1) + b if (t < 1)
      t-=1
      return c/2 * (-(2 ** -10 * t) + 2) + b
    end

    # circular easing in - accelerating from zero velocity
    def ease_in_circ(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      t /= d
      return -c * (Math.sqrt(1 - t*t) - 1) + b
    end

    # circular easing out - decelerating to zero velocity
    def ease_out_circ(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      t /= d
      t-= 1
      return c * Math.sqrt(1 - t*t) + b
    end

    # circular easing in/out - acceleration until halfway, then deceleration
    def ease_in_out_circ(t, b, c, d)
      t, b, c, d = t.to_f, b.to_f, c.to_f, d.to_f
      t /= d/2
      return -c/2 * (Math.sqrt(1 - t*t) - 1) + b if (t < 1)
      t -= 2
      return c/2 * (Math.sqrt(1 - t*t) + 1) + b
    end


  end
end