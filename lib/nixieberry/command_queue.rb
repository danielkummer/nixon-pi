require 'thread'
require_relative 'control_parameters'



module NixieBerry
  module CommandQueue
    include ControlParameters


    $queues = {}

    def enqueue(worker, params)
      puts "worker: #{worker}"
      puts "params: #{params}"
      command = control_parameters(worker).merge(params)
      command[:time] =  Time.now
      queue(worker) <<  command
      puts "command: #{command}"
      puts "queues: #{$queues}"
    end

    def queue(name)
      $queues[name] ||= Queue.new
    end

  end
end