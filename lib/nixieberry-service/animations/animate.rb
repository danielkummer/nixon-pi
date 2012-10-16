require 'thread'

module NixieBerry
  class Animate
    include AnimationQueue
    include NixieLogger

    def initialize
      @driver = AbioCardClient.instance
      @semaphore = Mutex.new
    end

    def animate
      @semaphore.synchronize {
        @t = Thread.new do
          @animations.each do |animation|
            queue << animation
          end
        end
      }
    end
  end
end