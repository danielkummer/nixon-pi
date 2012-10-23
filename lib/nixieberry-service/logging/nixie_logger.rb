require 'logger'

module NixieBerry
  module NixieLogger
    extend NixieConfig

    @loggers = {}

    def log
      @logger ||= NixieLogger.logger_for(self.class.name)
    end

    class << self
      def logger_for(classname)
        @loggers[classname] ||= configure_logger_for(classname)
      end

      def configure_logger_for(classname)
        path = File.join(Dir.pwd, 'nixie.log')
        logger = Logger.new MultiDelegator.delegate(:write, :close).to(STDOUT, File.open(path, "a"))
        logger.level = eval "Logger::#{config[:log_level]}"
        logger.progname = classname
        logger.formatter = proc do |severity, datetime, progname, msg|
          "[#{severity}] #{progname} -- #{datetime.strftime("%Y-%m-%d %H:%M:%S")}: #{msg}\n"
        end
        logger
      end
    end
  end
end