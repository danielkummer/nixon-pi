require 'rufus/scheduler'

module NixonPi
  class Scheduler
    include Logging
    include NixonPi::Commands
    include InformationHolder

    accepted_commands :method, :timing, :target, :command, :time, :id, :delete

    def initialize
      @@scheduler ||= Rufus::Scheduler.new
      @@jobs = {}

      def @@scheduler.handle_exception(job, exception)
        log.error "job #{job.job_id} caught exception '#{exception}'"
      end

      log.info 'Scheduler started'
      reload
    end

    def reload
      # delete all ambiguous records
      # Schedule.find(:all, conditions: ["method IN (?)", %w(in every)])
      schedules = Schedule.all
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
          ret = { jobs: jobs }
        else
          log.error "No information about #{about}"
      end
      ret
    end

    def handle_command(command)
      if command.key?(:delete)
        id = command[:id]
        log.info "deleting schedule with id: #{id}"
        unschedule(id)
      else
        log.info "got schedule command: #{command}, applying..."
        id = command[:id]
        method = command[:method]
        timing = command[:timing]
        target = command[:target]

        new_commands = {}
        command[:command].each do |k, v|
          new_commands[k.to_sym] = v
        end

        command[:command] = new_commands

        locked = command[:lock] ? true : false
        schedule(id, method, timing, target, command[:command], locked)
      end
    end

    def self.exit_scheduler
      # unschedule_all #quit cron jobs too...
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
      # if %w"in at".include?(method) or lock
      if lock
        log.debug 'locking state machine...'
        # TODO: lock queue
        # CommandQueue.lock(queue)
      end

      # TODO: test if block works
      job = case method.to_sym
              when :in
                @@scheduler.in "#{timing}", CommandJob.new(id, target, command, lock), mutex: "#{target}"
              when :at
                @@scheduler.at "#{timing}", CommandJob.new(id, target, command, lock), mutex: "#{target}"
              when :every
                @@scheduler.every "#{timing}", CommandJob.new(id, target, command, lock), mutex: "#{target}"
              when :cron
                @@scheduler.cron "#{timing}", CommandJob.new(id, target, command, lock), mutex: "#{target}"
              else
                false
            end

      @@jobs[id.to_s.to_sym] = job
      true
    end

    def unschedule(id)
      @@jobs[id.to_s.to_sym].unschedule unless @@jobs[id.to_s.to_sym].nil?
    end

    # unused
    def unschedule_all
      @@jobs.each(&:unschedule)
    end
  end
end
