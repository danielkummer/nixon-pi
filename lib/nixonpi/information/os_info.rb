##
# Detect OS
#
module OSInfo
  def OSInfo.windows?
    (/cygwin|mswin|mingw|bccwin|wince|emx/ =~ RUBY_PLATFORM) != nil
  end

  def OSInfo.mac?
    (/darwin/ =~ RUBY_PLATFORM) != nil
  end

  def OSInfo.unix?
    !OSInfo.windows?
  end

  def OSInfo.linux?
    OSInfo.unix? and not OSInfo.mac?
  end
end