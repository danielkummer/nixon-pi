# Remote procedure calls via Drb

The web service uses Drb to remotely communicate with objects holding information.
The objects with respond to the requests must implement handle_info_request via the InformationHolder module.

On the opposite site is the InformationProxy class which responds to the get_info_from request issued from the web service.
The proxy calls the handle_info_request method on the appropriate target and passes the about hash.

# RabbitMQ Processing

To process commands an object must implement the handle_command method and include the Commands module.

The class can then expose commands via accepted_commands on class level, example:

	accepted_commands :state, :value, :animation_name, :options
	
The central distribution mechanism for commands is the CommandReceiver class. It acts as a Bunny RabbitMQ client
for processing queue commands.

The class implements a locking mechanism for providing protection agains accessing a locked receiver
Every time a valid command is received, the class calls handle_command on the object

The whole controlling revolves around this mechanism.


