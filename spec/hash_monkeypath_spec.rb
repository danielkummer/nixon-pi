require_relative 'spec_helper'
require_relative '../lib/nixonpi/hash_monkeypatch'

describe Hash do
  it 'should have the string key to sym method' do
    {}.should respond_to :string_key_to_sym
  end

  it 'should convert all string keys to symbols in a hash' do
    input = { 'hello' => 'world', 'how' => 'are you' }
    expected = { hello: 'world', how: 'are you' }

    input.string_key_to_sym.should eq expected
  end
end
