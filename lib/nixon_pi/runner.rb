require 'em/connection'

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
    def self.run(opts = {})
      EventMachine.run do
        server = opts[:server] || 'thin'
        host = opts[:host] || '0.0.0.0'
        port = opts[:port] || '8181'
        web_app = NixonPi::WebServer.new
        dispatch = Rack::Builder.app { map('/') { run web_app } }

        raise "Need an EM webserver, but #{server} isn't" unless ['thin', 'hatetepe', 'goliath'].include? server


        Signal.trap('INT') { EventMachine.stop }
        Signal.trap('TERM') { EventMachine.stop }

        # Start the web server. Note that you are free to run other tasks
        # within your EM instance.
        Rack::Server.start({
                               app: dispatch,
                               server: server,
                               Host: host,
                               Port: port,
                               signals: false,
                           })


        EventMachine.defer do
          server = NixonPi::NixieService.new
          server.run!
        end

        EM.add_periodic_timer(1) do
         puts "Tick ... "
         end

        #EventMachine.next_tick do
        #  $stderr.puts "stderr before"
        #  EventMachine.popen3 cmd, MyProcess
        #  $stderr.puts "stderr after"
        #end

      end
    end
  end

end