##
# Detect OS
#
module OSInfo
  def self.windows?
    (/cygwin|mswin|mingw|bccwin|wince|emx/ =~ RUBY_PLATFORM) != nil
  end

  def self.mac?
    (/darwin/ =~ RUBY_PLATFORM) != nil
  end

  def self.unix?
    !OSInfo.windows?
  end

  def self.linux?
    OSInfo.unix? && !OSInfo.mac?
  end

  def self.network
    res = Socket.ip_address_list.select { |intf| intf.ipv4? && !intf.ipv4_loopback? && !intf.ipv4_multicast? }.collect(&:ip_address)
    res
  end
end
