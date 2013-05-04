require 'open3'
require 'thread'

module NixonPi
  class DirectIO

    def initialize
      @stdin, @stdout, @stderr, @wait_thr = Open3.popen3("sudo #{Dir.pwd}/c-driver/abiocard/abiocardserver -cl")
      #pid = @wait_thr[:pid]
      @mutex = Mutex.new
    end

    ##
    # Write to direct stdin and get values from stdout
    # @param [String] value
    # Pass a block if you like to handle the return value
    def cmd(value)
      @mutex.synchronize do
        @stdin.puts(value)
        if block_given?
          yield @stdout.gets
        end
      end
    end

    # Close io connections
    def close
      @mutex.synchronize do
        @stdin.puts("QU")
        @stdin.close
        @stdout.close
        @stderr.close
      end
      exit_status = @wait_thr.value # Process::Status object returned.
    end
  end
end