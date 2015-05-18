require 'open3'
require 'thread'

# one way to use io/wait:
# http://illuminatedcomputing.com/posts/2011/10/piping-in-ruby-with-popen3/

module NixonPi
  class DirectIO
    include Logging

    def initialize
      @stdin, @stdout, @stderr, @wait_thr = Open3.popen3('sudo /opt/abiocard/abiocardserver -stdio')
      # pid = @wait_thr[:pid]
      @@mutex = Mutex.new
      @errors = []
    end

    ##
    # Write to direct stdin and get values from stdout
    # @param [String] value
    # Pass a block if you like to handle the return value
    def cmd(value)
      log.debug("CMD: #{value}")
      @@mutex.synchronize do
        begin
          @stdin.puts(value)
          yield @stdout.gets if block_given?
        rescue => e
          raise e, @stderr.read
        end
        error = @stderr.read
        fail error unless error.blank?
      end
    end

    # Close io connections
    def close
      @@mutex.synchronize do
        @stdin.puts('QU')
        @stdin.close
        @stdout.close
        @stderr.close
      end

      exit_status = @wait_thr.value # Process::Status object returned.
    end
  end
end
