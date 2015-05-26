require 'open3'
require 'thread'

# one way to use io/wait:
# http://illuminatedcomputing.com/posts/2011/10/piping-in-ruby-with-popen3/

module NixonPi
  class DirectIO
    include Logging

    CRLF = "\x0d\x0a"

    def initialize
      @@io_mutex = Mutex.new
      @stdin, @stdout, @stderr, @wait_thr = Open3.popen3(Settings.abiocardserver_command)
    end

    ##
    # Write to direct stdin and get values from stdout
    # @param [String] value
    # Pass a block if you like to handle the return value
    def cmd(value)
      #log.debug("CMD: #{value}")
      @@io_mutex.synchronize do
        begin
          @stdin.puts("#{value}#{CRLF}")
          yield @stdout.gets if block_given?
        rescue => e
          raise e, @stderr.read
        end
      end
    end

    # Close io connections
    def close
      @@io_mutex.synchronize do
        @stdin.puts('QU')
        @stdin.close
        @stdout.close
        @stderr.close
      end
      #exit_status = @wait_thr.value # Process::Status object returned.
    end
  end
end
