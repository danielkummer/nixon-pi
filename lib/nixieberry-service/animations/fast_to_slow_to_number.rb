
class FastToSlowToNumber < Animate

  def initialize (start_value, totalduration)
    value = start_value.to_i
    add_to = ((value.size - 1) * 11)


   #
  #  Ah, yes. I should have known. It's basically duty cycle=sqrt(2)^x except scaled so that it gives the right domain for whatever range you want to use. (in this case scaled by about 1.408 or 255/sqrt(2)^16. You could use the form PWM=(range/sqrt(2)^domain)*sqrt(2)^x simplified PWM = range* 2^((x-domain)/2) I think for now I'll just calculate data for a larger domain and use that. I could have it calculate in real time, but look-up tables are faster and I already have it implemented. Thank you!
    #
    #

    value = value.to_i + add_to
    value = value % (value.size * 10)
    @animations << {type: :tube, value: value.to_s }

    #calculate_†imings
          #todo    0.5 - Math.cos( p*Math.PI ) / 2

    #back to start value...
    @animations << {type: :tube, value: start_value.to_s }
  end

  def animate

  end

end