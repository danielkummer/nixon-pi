require 'rufus/scheduler'

module NixonPi
  #module Scheduler
    class CommandJob
      include NixonPi::Logging
      include NixonPi::Messaging

      def initialize(id, target, command, lock)
        @id = id
        @target = target
        @command = command
        @lock = lock
      end

      def call(job)
        log.info "Job called#{job} "
        log.error "Error in job #{job}, queue: #{queue}, command: #{@command}" if @target.nil? || @command.nil?
        CommandSender.new.send_command(@target, @command)
        # TODO: unlock queue
        # CommandQueue.unlock(@queue) if @lock
        Schedule.delete(@id) if job.is_a?(Rufus::Scheduler::InJob) || job.is_a?(Rufus::Scheduler::AtJob)
      end
    end

  #end
end