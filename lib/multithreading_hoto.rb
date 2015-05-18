class Arduino
  def initialize
    @threads    = []
    @work_queue = Queue.new
    @mutex      = Mutex.new
    @cmd_lock   = ConditionVariable.new
    @serial     = Queue.new

    # Connect to arduino serial
    start_queues
  end

  def start_queues
    # read from serial
    @threads << Thread.new do
      loop do
        process(@serial.pop)
      end
    end

    # pop from queue to send to Arduino
    @threads << Thread.new do
      loop do
        @mutex.synchronize do
          send(@work_queue.pop)

          @cmd_lock.wait(@mutex)
        end
      end
    end
  end

  def process(data)
    puts data

    # Check cmd
    @cmd_lock.signal if data == 'example1'
  end

  def send(data)
    @serial << data
  end

  def go
    puts 'GO'
    @work_queue << 'example1'
    @work_queue << 'example2'
    @work_queue << 'example3'

    @threads.each(&:join)
  end
end

Arduino.new.go
