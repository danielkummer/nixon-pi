require 'em/connection'

# Override PeriodicTimer
module EventMachine
  class PeriodicTimer
    alias :old_initialize :initialize

    def initialize interval, callback=nil, &block
      # Added two additional instance variables to compensate difference.
      @start = Time.now
      @fixed_interval = interval
      old_initialize interval, callback, &block
    end

    alias :old_schedule :schedule

    def schedule
      # print "Started at #{@start}..: "
      compensation = (Time.now - @start) % @fixed_interval
      @interval = @fixed_interval - compensation
      # Schedule
      old_schedule
    end
  end
end

module EventMachine
  def self.popen3(*args)
    new_stderr = $stderr.dup
    rd, wr = IO::pipe
    $stderr.reopen wr
    connection = EM.popen(*args)
    $stderr.reopen new_stderr
    EM.attach rd, Popen3StderrHandler, connection
    connection
  end

  class Popen3StderrHandler < EventMachine::Connection
    def initialize(connection)
      @connection = connection
    end

    def receive_data(data)
      @connection.receive_stderr(data)
    end
  end
end

class MyProcess < EventMachine::Connection
  def initialize *args
    log __method__, args
  end

  def receive_data data
    log __method__, data
  end

  def receive_stderr data
    log __method__, data
  end

  def unbind
    log __method__
  end
end

module NixonPi
  class Runner
    include Logging

    def initialize
      @service = NixonPi::NixieService.new
    end

    def shutdown!
      log.info 'Shutting down...'
      @service.shutdown
      EM.stop
    end

    def run(opts = {})
      EventMachine.run do
        server = opts[:server] || 'thin'
        host = opts[:host] || '0.0.0.0'
        port = opts[:port] || '8181'
        web_app = NixonPi::WebServer.new
        dispatch = Rack::Builder.app { map('/') { run web_app } }

        raise "Need an EM webserver, but #{server} isn't" unless ['thin', 'hatetepe', 'goliath'].include? server

        trap('TERM') do
          Thread.new { shutdown! }
        end
        trap('INT') do
          Thread.new { shutdown! }
        end
        trap('QUIT') do
          Thread.new { shutdown! }
        end
        EventMachine.set_timer_quantum 10


        # Start the web server. Note that you are free to run other tasks
        # within your EM instance.
        Rack::Server.start({
                               app: dispatch,
                               server: server,
                               Host: host,
                               Port: port,
                               signals: false,
                           })


=begin
http://dev.af83.com/2011/09/20/fighting-with-eventmachine.html
http://dev.af83.com/2011/09/20/fighting-with-eventmachine.html

operation = proc {
  # perform a long-running operation here, such as a database query.
  "result" # as usual, the last expression evaluated in the block will be the return value.
}
callback = proc {|result|
  # do something with result here, such as send it back to a network client.
}

EventMachine.defer(operation, callback)
=end

        #TODO there is no nice way to shut down the system because the service spwawns seperate threads
        EventMachine.defer do
          @service.run!
        end
=begin
        server = NixonPi::NixieService.new
        server.run!


        EM.add_periodic_timer(1) do
          #puts "Tick ... "
          #  server = NixonPi::NixieService.new
          #  server.run!
          NixonPi::MachineManager.state_machines.each do |_type, state_machine|
            log.debug "Starting state machine: #{state_machine.class}"
            state_machine.handle
          end
        end
      #end

      #EventMachine.next_tick do
      #  $stderr.puts "stderr before"
      #  EventMachine.popen3 cmd, MyProcess
      #  $stderr.puts "stderr after"
      #end
=end

      end
    end
  end

end