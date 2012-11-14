require 'thread'


module NixieBerry
  module CommandQueue

    Command = Struct.new(:type, :params)

    $queues = {}

    def enqueue(worker, type, params = {})
      queue(worker) << Command.new(type, params)
    end

    def queue(name)
      $queues[:name] ||= Queue.new
    end

  end
end