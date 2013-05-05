require 'rufus/scheduler'
require 'singleton'
require_relative 'logging/logging'
require_relative 'messaging/commands_module'
require_relative 'messaging/command_receiver'
require_relative 'information/information_holder'

module NixonPi

  class CommandJob
    include Logging
    include Messaging

    def initialize(id, target, command, lock)
      @id, @target, @command, @lock = id, target, command, lock
    end

    def call(job)
      log.info "Job called#{job.to_s} "
      log.error "Error in job #{job.to_s}, queue: #{queue}, command: #@command" if @target.nil? or @command.nil?
      CommandSender.new.send_command(@target, @command)
      #todo unlock queue
      #CommandQueue.unlock(@queue) if @lock
      Schedule.delete(@id) if job.is_a?(Rufus::Scheduler::InJob) or job.is_a?(Rufus::Scheduler::AtJob)
    end
  end


  class Scheduler
    include Logging
    include Commands
    include InformationHolder

    accepted_commands :method, :timing, :target, :command, :time, :id, :delete

    def initialize
      @@scheduler ||= Rufus::Scheduler.start_new
      @@jobs = {}

      def @@scheduler.handle_exception(job, exception)
        self.log.error "job #{job.job_id} caught exception '#{exception}'"
      end

      log.info "Scheduler started"
      reload
    end

    def reload
      #delete all ambiguous records
      #Schedule.find(:all, conditions: ["method IN (?)", %w(in every)])
      schedules = Schedule.find(:all)
      schedules.each do |s|
        schedule(s.id, s.method, s.timing, s.target, s.command)
      end
    end

    def handle_info_request(about)
      ret = {}
      case about.to_sym
        when :commands
          ret = self.class.available_commands
        when :jobs
          ret = {jobs: jobs}
        else
          log.error "No information about #{about}"
      end
      ret
    end


    def handle_command(command)

      if command.has_key?(:delete)
        id = command[:id]
        log.info "deleting schedule with id: #{id}"
        unschedule(id)
      else
        log.info "got schedule command: #{command}, applying..."
        id, method, timing, target = command[:id], command[:method], command[:timing], command[:target]

        new_commands = Hash.new
        command[:command].each do |k, v|
          new_commands[k.to_sym] = v
        end

        command[:command] = new_commands

        locked = command[:lock] ? true : false
        schedule(id, method, timing, target, command[:command], locked)
      end
    end

    def self.exit_scheduler
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
    # @param [Symbol] target Name of the queue -> command receiver
    # @param [Hash] command Hash of command parameters
    def schedule(id, method, timing, target, command, lock = false)

      log.debug "schedule command #{command}, #{method} #{timing} for #{target}"
      #if %w"in at".include?(method) or lock
      if lock
        log.debug "locking state machine..."
        #todo lock queue
        #CommandQueue.lock(queue)
      end

      #todo test if block works
      job = case method.to_sym
              when :in
                @@scheduler.in "#{timing}", CommandJob.new(id, target, command, lock), :mutex => "#{target}"
              when :at
                @@scheduler.at "#{timing}", CommandJob.new(id, target, command, lock), :mutex => "#{target}"
              when :every
                @@scheduler.every "#{timing}", CommandJob.new(id, target, command, lock), :mutex => "#{target}"
              when :cron
                @@scheduler.cron "#{timing}", CommandJob.new(id, target, command, lock), :mutex => "#{target}"
              else
                false
            end


      @@jobs[id.to_s.to_sym] = job
      true
    end

    def unschedule(id)
      @@jobs[id.to_s.to_sym].unschedule unless @@jobs[id.to_s.to_sym].nil?
    end

    #unused
    def unschedule_all
      @@jobs.each do |j|
        j.unschedule
      end
    end
  end


end