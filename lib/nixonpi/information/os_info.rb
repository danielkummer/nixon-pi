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
end
