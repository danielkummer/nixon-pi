class Object
  # Checks if object is blank
  # @return [Boolean] true if empty, false otherwise
  def blank?
    respond_to?(:empty?) ? empty? : !self
  end
end
