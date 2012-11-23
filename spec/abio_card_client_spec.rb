require_relative 'spec_helper'
require_relative '../lib/nixieberry/client/abio_card_client'

describe NixonPi::AbioCardClient do

  before :each do
    @client = NixonPi::AbioCardClient.instance
  end

  it "should get a valid singleton instance" do
    @client.should_not be_nil
  end

  it "should raise an error when reading adc values" do
    lambda { @client.read_adc(1) }.should raise_error NotImplementedError
  end

  it "should write the correct value to the rtc clock" do
    time = Time.new
    @client.clock_write(time)
    $last_cmd.should include time.strftime("%y%m%d%H%M%S")
  end

  it "should correctly read the rtc clock value" do
    time = @client.clock_read
    time.should eq Time.new(2011, 11, 11, 11, 11, 11, "+01:00")
  end

  it "should read the abiocard hardware information" do
    hi = @client.info
    hi.rtc.should eq "1"
    hi.io.should eq "1"
    hi.adc.should eq "1"
    hi.pwm.should eq "1"
  end

  it "should write 1 to the io port 0" do
    @client.io_write(0, 1)
    $last_cmd.should eq "EW01"
  end

  it "should write multiple values to multiple ports while preserving the register state" do
    @client.io_write(1, 1)
    $last_cmd.should eq "EW03"
    @client.io_write(2, 1)
    $last_cmd.should eq "EW07"
  end

  it "should read from the io ports" do
    8.times.with_index do |i|
      val =@client.io_read(i)
      val.should eq 1
    end
  end

  it "should write to pwm registers" do
    @client.pwm_write(0, 124)
    $last_cmd.should eq "PW00107C000000000000000000000000000000"
  end

  it "should reset all pwm registers" do
    @client.pwm_reset
    $last_cmd.should eq "PW001000000000000000000000000000000000"
  end

  it "should globally dim the pwm registers" do
    @client.pwm_global_dim(124)
    $last_cmd.should eq "PW10017C"
    @client.pwm_global_dim(255)
    $last_cmd.should eq "PW1001FF"
  end

  it "should read from all pwm registers" do
    expected = Array.new(17, 255) #0..15 pwm, 16 global pwm
    actual = @client.send(:pwm_read_registers)
    actual.should eq expected
  end

end