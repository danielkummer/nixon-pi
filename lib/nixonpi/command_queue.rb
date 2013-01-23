require_relative 'logging/logging'
require 'bunny'
require 'json'


#todo optimize massively

module NixonPi
  class CommandQueue
    extend Logging

    @@listeners = {}
    @@locked = {}
    @@queues = []

    class << self









      def add_receiver(receiver, queue)
        raise "Receiver must include the receiver module" unless receiver.is_a?(CommandReceiver)

        if @@listeners[queue.to_sym].nil?
          @@listeners[queue.to_sym] = [receiver]
        else
          @@listeners[queue.to_sym] << receiver
        end


        #result = self.channel.queue_declare(exclusive=True)
        #self.callback_queue = result.method.queue
        #client.queue

        q = client.queue(queue, :auto_delete => true).bind(direct_exchange, :routing_key => queue)
        #q = client.queue("", :auto_delete => true).bind(direct_exchange, :routing_key => queue)
        #q.subscribe(:block => true, :ack => true)
        q.subscribe do |delivery_info, properties, payload|

          # FYI: there is a shortcut, Bunny::Channel.ack
          #direct_exchange.acknowledge(delivery_info.delivery_tag, false)

          command = string_key_to_sym(JSON.parse(payload))

          if command[:time] and Time.parse(command[:time]) + 2 > Time.now #do nothing if command is older than 2 seconds
            log.debug "processing command: #{command} in queue #{queue}, checking for invalid control parameters..."
            @@listeners[queue].each do |listener|

              #todo there might be a better place to handle invalid commands, maybe when enqueuing them?
              command.delete_if { |k, v| !listener.class.available_commands.include?(k) or v.nil? }

              if listener.respond_to?(:receive)
                listener.try(:receive, command)
                #todo add rescue (maybe)
              else
                log.error "Listener for #{queue} doesn't have the receive method!"
              end
            end
          else
            log.error "command too old - don't process..."
          end


        end

        @@queues << q
      end

      def string_key_to_sym(hash)
        result = Hash.new
        hash.each { |k, v| result[k.to_sym] = v }
        result
      end

      def get_receiver_for(queue)
        @@listeners[queue.to_sym]
      end


      #deprecate :lock, :unlock, :locked?

      ##
      # Enqueue a new command
      # @param [Symbol] worker
      # @param [Hash] params
      def enqueue(worker, params)
        worker = worker.to_sym
        command = params.clone
        if can_enqueue?(worker, command)

          command[:time] = Time.now
          log.info "Enqueueing #{command.to_s} for #{worker}"

          direct_exchange.publish(command.to_json, :routing_key => worker)

        else
          log.error "Queue currently locked, cannot enqueue #{params.to_s} for #{worker}"
        end
      end




      def lock(worker)
        @@locked[worker] = true
      end

      def unlock(worker)
        @@locked[worker] = false
      end

      def locked?(worker)
        @@locked[worker] ? true : false
      end

      def on_exit
        @@queues.each do |q|
          q.delete
        end
        direct_exchange.delete
        client.close

      end


      private
      #todo the whole locking might not work when using rabbitmq
      def can_enqueue?(worker, params)
        if locked?(worker) and params[:priority] == true
          true
        elsif locked?(worker)
          false
        else
          true
        end
      end


      #https://github.com/rabbitmq/rabbitmq-cloudfoundry-samples/blob/master/sinatra/rabbit.rb
      def client
        unless $client
          conn = Bunny.new
          conn.start
          $client = conn
          #$client.qos :prefetch_count => 1
        end
        $client
      end

      def direct_exchange
        $direct_exchange ||= client.create_channel.direct("nixonpi")
      end


    end
  end
end