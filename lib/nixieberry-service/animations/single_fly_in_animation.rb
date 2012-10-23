=begin
every single char of the number flies in from the defined site to the correct position

ex:
  123456
-> 6 ..6
-> 5 .56


__1234
4_____
_4____
__4___
___4__
____4_
_____4
3____4
_3___4
__3__4
___3_4
____34
=end
class SingleFlyInAnimation < TubeAnimation

  register_animation :single_fly_in

  def initialize(options)
    @options = options
  end


  def run
    @t = Thread.new do
      options = @options
      write_values(options[:start_value], options[:sleep])
    end
    @driver = TubeDriver.instance.write(start_value)
  end
end

private
def write_values(start_value, sleep_duration)
  original_length = start_value.length
  pad_times = original_length
  first_number_position = start_value.index(/\d/)
  last_output_of_number = ""
  append_number = ""

  start_value.reverse.each_char do |number|
    pad_times.times do |current|
      current_output = ""
      current_output << "_" * (current)
      current_output << number.to_s

      current_output << "_" * (original_length - current - append_number.length - 1)
      current_output << append_number
      puts current_output + "end"

      @driver = TubeDriver.instance.write(current_output)
      last_output_of_number = current_output
      sleep sleep_duration
    end
    append_number = last_output_of_number[pad_times - 1] + append_number
    pad_times = pad_times - 1
    break if pad_times == first_number_position
  end
end

