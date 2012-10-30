require 'logger'
require_relative '../../nixieberry/configurations/settings'
require_relative '../delegators/multi_delegator'

module NixieBerry
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
        path = File.join(Dir.pwd, 'nixie.log')
        logger = Logger.new MultiDelegator.delegate(:write, :close).to(STDOUT, File.open(path, "a"))
        logger.level = eval "Logger::#{Settings.log_level}"
        logger.progname = classname
        logger.formatter = proc do |severity, datetime, progname, msg|
          "[#{severity}] #{progname} -- #{datetime.strftime("%Y-%m-%d %H:%M:%S")}: #{msg}\n"
        end
        logger
      end
    end
  end
end