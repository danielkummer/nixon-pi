require 'logger'
require_relative '../../nixonpi/configurations/settings'
require_relative '../delegators/multi_delegator'

class String
  # colorization
  def colorize(color_code)
    "\e[#{color_code}m#{self}\e[0m"
  end

  def red
    colorize(31)
  end

  def green
    colorize(32)
  end

  def yellow
    colorize(33)
  end

  def pink
    colorize(35)
  end
end

module NixonPi
  module Logging
    @loggers = {}

    ##
    # Get the logger for the current class
    def log
      @logger ||= Logging.logger_for(self.class.name)
    end

    class << self
      def logger_for(classname)
        @loggers[classname] ||= configure_logger_for(classname)
      end

      def configure_logger_for(classname)
        path = File.join(Dir.home, 'nixon-pi.log')
        logger = Logger.new MultiDelegator.delegate(:write, :close).to(STDOUT, File.open(path, 'a'))
        logger.level = eval "Logger::#{Settings.log_level}"
        logger.progname = classname
        logger.formatter = proc do |severity, datetime, progname, msg|
          format = "[#{severity}] #{progname} -- #{datetime.strftime('%Y-%m-%d %H:%M:%S')}: #{msg}\n"
          case severity
            when 'INFO'
              format = format.green
            when 'ERROR'
              format = format.red
            when 'DEBUG'
              format = format.yellow
            else
              format
          end
          format
        end
        logger
      end
    end
  end
end
