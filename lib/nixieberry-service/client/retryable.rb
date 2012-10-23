module Retryable
##
# If retry times more than retry times in option parameter, will raise a RetryError.
# * :retry_times - Retry times , Defaults 10
# * :on - The Exception on which a retry will be performed. Defaults Exception
# Notice: This method will call block many times, so don't put can't retryable code in it.
# Example
# =======
#    begin
#      retryable_proxy(:retry_times => 10,:on => Timeout::Error) do |ip,port|
#         # your code here
#      end
#    rescue RetryError
#      # handle error
#    end
#
  def retryable(options = {})
    opts = {:retry_times => 10, :on => Exception}.merge(options)
    retry_times, try_exception = opts[:retry_times], opts[:on]
    begin
      yield
    rescue try_exception => e
      if (retry_times -= 1) > 0
        log.info "retrying another #{retry_times} times"
        retry
      end
      log.info "failed retrying... #{e.message}"
      raise RetryError
    end
  end
end
