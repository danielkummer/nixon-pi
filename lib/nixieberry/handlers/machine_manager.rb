require_relative 'handler_state_machine'

module NixieBerry
  class MachineManager

    @@state_machines, @@threads = [], []

    def self.add_state_machines(*types)
      types.each do |type|
        @@state_machines << NixieBerry::HandlerStateMachine.create(type)
      end
    end


    def self.start_state_machines(sleep_for_sec = 0.3)
      @@state_machines.each do |state_machine|
        @@threads << Thread.new do
          loop do
            #puts "tsm active"
            state_machine.handle
            sleep sleep_for_sec #tacted at 100ms, adjust if necessary
          end
        end
      end
    end

    def self.on_exit
      @@threads.each { |thread| thread.join }
    end

  end
end