class Hash
  ##
  # Convert string keys to symbols
  # TODO : Refactor this away
  def string_key_to_sym
    result = {}
    each { |k, v| result[k.to_sym] = v }
    result
  end
end
