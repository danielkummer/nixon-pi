require 'open3'

module NixonPi
  class DirectIO

    def initialize
      @stdin, @stdout, @stderr, @wait_thr = Open3.popen3("#{Dir.pwd}/abiocard/abiocardserver -cl")
      #pid = @wait_thr[:pid]
    end

    ##
    # Write to direct stdin and get values from stdout
    # @param [String] cmd command
    # Pass a block if you like to handle the return value
    def cmd(value)
      @stdin.puts(value)
      if block_given?
        yield @stdout.gets
      end
    end

    # Close io connections
    def exit
      @stdin.close
      @stdout.close
      @stderr.close
      exit_status = @wait_thr.value # Process::Status object returned.
      exit_status
    end
  end
end