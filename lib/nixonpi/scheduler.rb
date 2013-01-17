require 'rufus/scheduler'
require 'singleton'
require_relative 'command_processor'
require_relative 'logging/logging'

module NixonPi

  class CommandJob
    include Logging

    def initialize(id, queue, command, lock)
      @id, @queue, @command, @lock = id, queue, command, lock
    end

    def call(job)
      log.info "Job called#{job.to_s} "
      CommandQueue.enqueue(@queue, @command)
      CommandQueue.unlock(@queue) if @lock
      Schedule.delete(@id)
    end
  end


  class Scheduler
    include Logging
    include Singleton

    def initialize
      #https://github.com/jmettraux/rufus-scheduler
      @@scheduler ||= Rufus::Scheduler.start_new
      @@jobs = {}

      def @@scheduler.handle_exception(job, exception)
        log.error "job #{job.job_id} caught exception '#{exception}'"
      end

      log.info "Scheduler started"
    end

    def reload_from_db
      #delete all ambiguous records
      Schedule.find(:all, conditions: ["method IN (?)", %w"in every"])
    end


    def receive(command)
      log.debug "got schedule command: #{command}, applying..."
      id, method, timing, queue = command[:id], command[:method], command[:timing], command[:queue]

      new_commands = Hash.new
      command[:command].each do |k, v|
        new_commands[k.to_sym] = v
      end

      command[:command] = new_commands

      locked = command[:lock] ? true : false;
      schedule(id, method, timing, queue, command, locked)
    end

    def exit_scheduler
      #unschedule_all #quit cron jobs too...
    end

    ##
    # Get a list of all jobs
    # @@return
    def jobs
      @@scheduler.all_jobs
    end

    def running_jobs
      @@scheduler.running_jobs
    end

    ##
    # Schedule a command to be executed
    # @param [Symbol] method Type can be in, at, cron, every
    # @param [String] timing Timestring, see documentation for possibilities
    # @param [Symbol] queue Name of the queue -> command receiver
    # @param [Hash] command Hash of command parameters
    def schedule(id, method, timing, queue, command, lock = false)
      if %w"power tubes bars lamps".include?(queue)
        log.debug "schedule command #{command}, #{method} #{timing} for #{queue}"
        #if %w"in at".include?(method) or lock
        if lock
          log.debug "locking state machine..."
          CommandQueue.lock(queue)
        end

        #todo test if block works
        job = case method.to_sym
                when :in
                  @@scheduler.in "#{timing}", CommandJob.new(id, queue, command, lock), :mutex => "#{queue}"
                when :at
                  @@scheduler.at "#{timing}", CommandJob.new(id, queue, command, lock), :mutex => "#{queue}"
                when :every
                  @@scheduler.every "#{timing}", CommandJob.new(id, queue, command, lock), :mutex => "#{queue}"
                when :cron
                  @@scheduler.cron "#{timing}", CommandJob.new(id, queue, command, lock), :mutex => "#{queue}"
              end


        @@jobs[id.to_s.to_sym] = job
      end
    end
  end

  def unschedule(id)
    @@jobs[id.to_sym].unschedule unless @@jobs[id.to_sym].nil?
  end

  #unused
  def unschedule_all
    @@jobs.each do |j|
      j.unschedule
    end
  end

end