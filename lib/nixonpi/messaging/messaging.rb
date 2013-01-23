require_relative '../logging/logging'
require 'json'

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
          $client.qos :prefetch_count => 1 #todo maybe remove
        end
        $client
      end

      def direct_exchange_channel
        $direct_exchange ||= client.create_channel.direct("nixonpi")
      end
    end

    #server
    class Replier
      include Common

      def initialize
        @incoming_requests_queue = direct_exchange_channel.queue("", exclusive: true, auto_delete: true)
        @incoming_requests_queue.subscribe(:ack => true) do |metadata, payload|
          log.debug "[requests] Got a request #{metadata.message_id}. Sending a reply..."

          #todo get reply
          reply = "hello"

          #send the reply
          direct_exchange_channel.publish(reply.to_json,
                                          routing_key: metadata.reply_to,
                                          correlation_id: metadata.message_id,
                                          mandatory: true,
                                          immediate: true,
                                          content_type: 'application/json')
          #mark as handled...
          metadata.ack
        end

      end

    end

    #client
    class Requester
      include Common
      include Logging


      def initialize

        @replies_queue = client.queue("", :exclusive => true, :auto_delete => true).bind(direct_exchange_channel)
        @replies_queue.subscribe do |delivery_info, metadata, payload|
          log.debug "[response] Response for #{metadata.correlation_id}: #{payload.inspect}"
          #add callback method

        end

      end

      def request(queue, payload)

        #todo get and set type from payload (state transition or so...)

        log.info "Enqueueing #{payload.to_s} for #{queue}"
        direct_exchange_channel.publish(command.to_json,
                                        routing_key: queue,
                                        message_id: Kernel.rand(10101010).to_s,
                                        reply_to: @replies_queue.name,
                                        content_type: 'application/json')
        #block and wait for reply to come in...


=begin
while (true) {
        QueueingConsumer.Delivery delivery = consumer.nextDelivery();
        if (delivery.getProperties().getCorrelationId().equals(corrId)) {
            response = new String(delivery.getBody());
            break;
        }
    }
=end
      end

    end
  end
end
