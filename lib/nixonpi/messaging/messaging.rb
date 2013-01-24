require_relative '../logging/logging'
require_relative 'message_listener'
require 'json'
require 'singleton'

require 'bunny'

require_relative '../../nixonpi/hash_monkeypatch'

#http://rubydoc.info/github/ruby-amqp/amqp/master/file/docs/PatternsAndUseCases.textile
#http://rubydoc.info/github/ruby-amqp/amqp/master/file/docs/Exchanges.textile#Using_the_Publisher_Confirms_extension_to_AMQP_v0_9_1
#http://pubs.vmware.com/vfabric52/index.jsp?topic=/com.vmware.vfabric.rabbitmq.2.8/rabbit-web-docs/tutorials/tutorial-six-java.html

#http://www.rabbitmq.com/tutorials/tutorial-six-python.html (rpc)

#http://www.thesoftwaresimpleton.com/blog/2012/04/10/rabbitmq-websockets/

#http://rubydoc.info/github/ruby-amqp/amqp/master/file/docs/Exchanges.textile#Using_the_Publisher_Confirms_extension_to_AMQP_v0_9_1

#http://rubydoc.info/github/ruby-amqp/amqp/master/file/docs/Queues.textile


#http://rubyamqp.info/articles/getting_started/

#http://rubyamqp.info/articles/patterns_and_use_cases/


#http://rubybunny.info/articles/queues.html#consumer_instances
#http://rubybunny.info/articles/extensions.html

module NixonPi
  module Messaging
    module Common
      def client
        unless $client
          conn = Bunny.new
          conn.start
          $client = conn
          #$client.qos :prefetch_count => 1 #todo maybe remove
        end
        $client
      end

      def direct_exchange_channel
        $direct_exchange ||= client.create_channel.direct("nixonpi.channel")
      end

      def on_exit
        direct_exchange_channel.delete
        #queues delete
        log.info "closing connection to rabbitmq..."
        client.close
      end
    end

    class MessageSender
      include Common
      include Logging

      def send_command(target, payload)
        log.info "sending #{payload.to_s} to #{target}"
        direct_exchange_channel.publish(payload.to_json,
                                        routing_key: "nixonpi.command",
                                        content_type: 'application/json',
                                        type: target)
      end

    end

    class InquirySender
      include Common
      include Logging

      def initialize
        @replies_queue = client.queue("", :exclusive => true, :auto_delete => true).bind(direct_exchange_channel)
        @replies_queue.subscribe(block: false) do |delivery_info, metadata, payload|
          log.debug "[response] Response for #{metadata.correlation_id}: #{payload.inspect}"
          #if @msg_id == metadata.correlation_id

          log.debug "message id match, parse payload"
          @reply = JSON.parse(payload)
          #end

        end
      end

      #this thing is blocking!!
      #todo refactor
      def get_info(from_target, about_what)
        target = from_target
        about = {what: about_what.to_s}
        log.info "Enqueueing #{about.to_s} for #{target}"
        @msg_id = Kernel.rand(10101010).to_s
        @reply = nil
        direct_exchange_channel.publish(about.to_json,
                                        message_id: @msg_id,
                                        reply_to: @replies_queue.name,
                                        content_type: 'application/json',
                                        type: target,
                                        routing_key: "nixonpi.inquiries")
        #routing_key: "nixonpi.inquire"
        #baad

        nil
      end

    end


    class MessageReceiver
      include Common
      include Logging


      def initialize
        @receivers = Hash.new
        subscribe()
      end

      def add_receiver(receiver, command)
        command = command.to_sym
        raise "Receiver must include the receiver module" unless receiver.is_a?(MessageListener)
        if @receivers[command].nil?
          @receivers[command] = [receiver]
        else
          @receivers[command] << receiver
        end

      end

      #todo refactor
      def subscribe()
        @incomming_commands_queue = client.queue("", exclusive: false, auto_delete: false).bind(direct_exchange_channel, routing_key: "nixonpi.command")
        @incomming_commands_queue.subscribe(consumer_tag: "command_consumer") do |delivery_info, metadata, data|
          if metadata[:content_type] == 'application/json'
            datahash = JSON.parse(data)
            handle_command_for(metadata.type, datahash.string_key_to_sym)
          else
            log.error("no hanlder for content type: #{metadata[:content_type]}")
          end

        end

        @incomming_inquiries_queue = client.queue("", exclusive: false, auto_delete: true).bind(direct_exchange_channel, routing_key: "nixonpi.inquiries")
        @incomming_inquiries_queue.subscribe(consumer_tag: "inquiry_consumer") do |delivery_info, metadata, data|
          log.debug "[requests] Got a request #{metadata.message_id}. Sending a reply..."


          if metadata[:content_type] == 'application/json'
            data_hash = JSON.parse(data)
            reply = handle_inquiry_for(metadata.type, data_hash.string_key_to_sym)
          else
            log.error("no hanlder for content type: #{metadata[:content_type]}")
          end

          reply ||= {success: false, message: "no respond to inquiry"}

          log.debug "replying with: #{reply.to_s}"
          #send the reply
          direct_exchange_channel.publish(reply.to_json,
                                          routing_key: metadata.reply_to,
                                          correlation_id: metadata.message_id,
                                          mandatory: true,
                                          immediate: true,
                                          content_type: 'application/json')
          # FYI: there is a shortcut, Bunny::Channel.ack

        end


      end

      def handle_inquiry_for(target, data)
        handle(:inquiry, target, data)
      end

      def handle_command_for(target, data)
        handle(:command, target, data)
      end

      private
      def handle(what, target, command)
        case what
          when :inquiry
            method = :handle_inquiry
          when :command
            method = :handle_command
          else
            raise ArgumentError
        end

        target = target.to_sym
        if @receivers.has_key?(target)

          @receivers[target].each do |receiver|
            if receiver.respond_to?(method)
              case method
                when :handle_inquiry
                  return receiver.try(method, command)
                else
                  command.delete_if { |k, v| !receiver.class.available_commands.include?(k) or v.nil? }
                  receiver.try(method, command)
              end
            else
              log.error "Listener for #{queue} doesn't have the receive method!"
            end
          end
        end
      end
    end
  end
end
