#currently unused
#@deprecated
module AnimationQueue
  def queue
     @queue ||= Queue.new
   end
end