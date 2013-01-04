require 'rufus/scheduler'
require 'command_queue'
require "singleton"
require_relative 'logging/logging'

module NixonPi
  class Scheduler
    include Singleton
    include Logging

    def initialize
      #https://github.com/jmettraux/rufus-scheduler
      @scheduler = Rufus::Scheduler.start_new

      def @scheduler.handle_exception(job, exception)
        log.error "job #{job.job_id} caught exception '#{exception}'"
      end
    end

    ##
    # Get a list of all jobs
    # @@return
    def jobs
      @scheduler.all_jobs
    end

    def running_jobs
      @scheduler.running_jobs
    end

    ##
    # Schedule a command to be executed
    # @param [Symbol] type Type can be in, at, cron, every
    # @param [String] time Timestring, see documentation for possibilities
    # @param [Symbol] queue Name of the queue -> command receiver
    # @param [Hash] command Hash of command parameters
    def schedule(type, time, queue, command)
      if %w"power tubes bars lamps".include?(type)
        log.debug "schedule command #{command}, in #{time} for #{type}"
        @scheduler.send(type, "#{time}", :mutex => "#{queue}") do
          CommandQueue.enqueue(queue, command)
        end
      end
    end
  end
end

=begin
  scheduler.at 'Thu Mar 26 07:31:43 +0900 2009' do
    puts 'order pizza'
  end

  scheduler.cron '0 22 * * 1-5' do
    # every day of the week at 22:00 (10pm)
    puts 'activate security system'
  end

  scheduler.every '5m' do
    puts 'check blood pressure'
  end

end
end
=end