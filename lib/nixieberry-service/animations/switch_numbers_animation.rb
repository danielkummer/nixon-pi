=begin
increments every number by 1 for the amount of time given (in seconds)

=end
class SwitchNumbersAnimation < TubeAnimation

  register_animation :switch_numbers

  def initialize(options)
    @options = options
  end

  def run
    duration = @options[:duration]
    sleep_step = 0.5 #seconds
    execution_times = duration / sleep_step
    value = @start
    @t = Thread.new do
      execution_times.times do
        value = self.try(:next_value, value)
        #todo call correct driver automatically
        @driver = TubeDriver.instance.write(value)
        sleep sleep_step
      end
      @driver = TubeDriver.instance.write(start_value)
    end
  end

  private
  def next_value(value)
    next_value = ""
    value.each_char do |number|
      number = number.to_i
      next_value << number + 1 % 10
    end
    next_value.to_s
  end
end