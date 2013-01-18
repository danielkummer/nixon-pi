# NIXON-&#928; :: The Raspberry Pi based Nixie Display


The Nixon-&#960; Display consists of **Nixie Tubes** provided by [Ogilumen](http://www.ogilumen.com/), driven by a **Raspberry Pi Model B** with an **I/O expansion card** from [Axiris](http://www.axiris.be/).
It's driven by a multi-threaded ruby application consisting of I/O drivers, state machines, a sinatra based web app and json api connected by message queues.

What started out as a private idea found it's way into a mini-project on a __LABS__ team event from [Namics](http://www.namics.com).

Many thanks to all who contributed and hopefully will contribute to this project!

**Jump to**

- [Components](#components)
- [Case](#case)
- [Installation](#installation)
- [Service](#service)
- [API](#api)


##<a id="components" name="components"></a>Components

The following main components were used for assembly, not mentioning wiring and tools.

### 1x Raspberry Pi Model B

<img src="http://www.raspberrypi.org/wp-content/uploads/2012/04/Raspi_Iso_Blue.png" width="400" height="150">
---
The [Raspberry Pi](http://www.raspberrypi.org/) is ideal for the project as it's a fully flegded mini computer which is capable of running all the things we need.

A standard installation with Debian Wheezy is recommended.
Install Ruby using the tutorial on [elinux](http://elinux.org/RPi_Ruby_on_Rails)

Here's a condensed list of the commands:

	sudo apt-get install -y git curl zlib1g-dev subversion
	sudo apt-get install -y openssl libreadline6-dev git-core zlib1g libssl-dev
	sudo apt-get install -y libyaml-dev libsqlite3-dev sqlite3
	sudo apt-get install -y libxml2-dev libxslt-dev
	sudo apt-get install -y autoconf automake libtool bison
	curl -L get.rvm.io | bash -s stable


### 1x [Axiris](http://www.axiris.be/) I/O Card for Raspberry Pi

<img src="http://www.axiris.be/en/images/stories/bcm2835/bcm2835_013_500.jpg" width="400" height="150">
---
Because the Raspberry Pi only has [limited IO capabilites](http://elinux.org/RPi_Low-level_peripherals) the [Axisis I/O card](http://www.axiris.be/en/index.php?option=com_content&view=article&id=51:i2c-io-card-for-use-with-raspberry-pi-computer&catid=14:io-cards&Itemid=34) is the perfect addition to fully satisfy our needs. It adds:

* a very high accuracy real-time clock based on the PCF2129A with CR2032 backup battery.
* 8 quasi bidirectional I/O lines (PCF8574), 5V levels.
* 8 12-bit ADC inputs (MAX11614EEE+), 0 - 4.096V range.
* 16 output channels providing 12-bit pulse-width modulation (PWM) output channels at about 40 to 1000 Hz with LED drive capability, 10 mA source, 25 mA sink, 5 V levels.

The repository contains a modified driver to allow direct console output - without the need to start a telnet server in order to control the expansion card.
However, you're still able to use the telnet server while developing - with it you can remotely execute the code with a live connection.

### 6x Nixie Duo Kits

<img src="http://www.ogilumen.com/images/product_pics/n2xdp1.jpg" width="400" height="150">
---

[Ogilumen](http://www.ogilumen.com/) sells beautiful, compact and easy to assemble nixie tube kits containing two IN-12A tubes each.
Some electronics are needed so we're able to drive them using the 12-bit PWM channels.

**Electric diagram**

<img src="https://dl.dropbox.com/u/23566127/nixon-pi_images/nixie_duo_diagram.png" width="400" height="150">


### 4x IN-13 Bar Graphs


<img src="http://www.ogilumen.com/images/product_pics/IN-13a.jpg" width="400" height="150">
---

The IN-13 Neon Bar Graphs sold by Ogilumen are a beautiful way to display a variable length glowing bar.

**Electric diagram**

<img src="https://dl.dropbox.com/u/23566127/nixon-pi_images/bargraph_diagram.png" width="400" height="150">


### 5x IN-1 Neon


<img src="http://www.ogilumen.com/images/product_pics/n1a.jpg" width="400" height="150">
---
The IN-1 Neon Lamps are simply a nice LED-alternative and they have a nice warm orange glow...

**Electric diagram**

<img src="https://dl.dropbox.com/u/23566127/nixon-pi_images/lamp_diagram.png" width="400" height="150">


## 1x 50 mA Nixie Tube Power Supply

<img src="http://www.ogilumen.com/images/product_pics/smps1.jpg" width="400" height="150">
---

In the end we need a power supply to drive all our high-voltage Nixie goodness. Luckly ogilumen provides us with a readily assembled power supply capable of providing all the power we need...
If you want to power everything with a 12V power supply, use a voltage divider and a modified micro-usb jack to power the raspberry pi.

**Voltage divider electric diagram**

<img src="https://dl.dropbox.com/u/23566127/nixon-pi_images/voltage_divider_diagram.png" width="400" height="150">

##<a id="case" name="case"></a> Case

The casing gives the nixon-pi a retro look - it should remind one of a sixties radio. A swiss carpentry manufactured the wooden casing - the rest was assembled by hand.

<img src="https://dl.dropbox.com/u/23566127/nixon-pi_images/front_plate.png" width="800">

## <a id="installation" name="installation"></a>Installation

Although nixon-pi can be packed as a gem it's currently suggested to install it by cloning the repository.
The master branch contains the newest production release.

##<a id="service" name="service"></a> The Service

A telnet based client to drive nixie tubes via rest interface

## Comand line options
	
	bin/nixon-pi -h
	Usage: nixon-pi [options]
	    -e env                           set the environment (default is development, others are test and production)
	    -m                               force the usage of the telnet mock interface
	    -p port                          set the webserver port (default is 8080)
	    -h, --help                       Display help

The application drops a .yml configuration file in the users home directory. Review and adjust if neccessary...    

### Manage the daemon process

    ruby bin/nixonpi.rb start
    ruby bin/nixonpi.rb restart
    ruby bin/nixonpi.rb stop

### Controlling

The daemon is controlled via sinatra web application, see the example page at [server ip]:[port]

## The Gem

The Nixon-&#960; Service is packed as a standalone gem which can be run as a daemon.

### Build

Build with

    gem build nixon-pi.gemspec


### Install

    $ gem install nixie-berry-service


### State machines

The service uses a state machine to handle local executions.
Generate state machine diagrams with the following command:


	rake state_machine:draw FILE=./lib/nixonpi/state_machines/tube_state_machine.rb CLASS=NixonPi::TubeStateMachine
	rake state_machine:draw FILE=./lib/nixonpi/state_machines/bar_state_machine.rb CLASS=NixonPi::BarStateMachine
	rake state_machine:draw FILE=./lib/nixonpi/state_machines/lamp_state_machine.rb CLASS=NixonPi::LampStateMachine

**Tube State Machine**

<img src="https://dl.dropbox.com/u/23566127/nixon-pi_images/TubeStateMachine_state.png" width="400">

**Bar State Machine**

<img src="https://dl.dropbox.com/u/23566127/nixon-pi_images/BarStateMachine_state.png" width="400">

**Lamp State Machine**

<img src="https://dl.dropbox.com/u/23566127/nixon-pi_images/LampStateMachine_state.png" width="400">
	
	
#<a id="api" name="api"></a> API

All api requests can be made as either .html or .json.
**JSONP** is returned to support 3rd party apps.

## Port

The web server port can be configured in the nixonpi-settings.yml file inside your home directory.

	prodiction:
	  web_server:
	    port: 3000


## POST Requests

Post requests to control the nixon-pi.

### POST /tubes

Control the IN-12A digit tubes.
The Tubes allow values from 0 to 9 including _ (Space) for no value.

| Parameters | Values | Data Type | Description  | Required |
| ---------- | ------ | --------- | ------------ | -------- |
| state | time, free_value, animation, countdown, run_test | String | State of the state machine | * |
| value | 0-9 or _ (Space) | String | Max. length 12 | * |
| time_format | ex: "%H%M%S" | String | only in state "time" | |
| animation_name | single_fly_in, count_from_to, switch_numbers | String |  | |
| options |  | JSON | Advanced options hash | |
| initial_mode | true, false | Boolean | Load on startup | |

### POST /lamps

Control the IN-1 neon tubes.
The neon tubes can either be turned on or off - they're like orange LEDs.

| Parameters | Values | Data Type | Description  | Required |
| ---------- | ------ | --------- | ------------ | -------- |
| state | free_value, animation, run_test | String |  State of the state machine  | * |
| values | Array of 0 or 1 | Array of Integer | Max. length 5 | * |
| animation_name | single_fly_in, count_from_to, switch_numbers | String |  | |
| options |  | JSON |Advanced options hash | |
| initial_mode | true, false | Boolean |Load on startup | |

### POST /bars

Control the IN-13 bargraph tubes.
The bargraphs are a linear display to visualize a value between 0 (off) and 255 (full width).

| Parameters | Values | Data Type | Description  | Required |
| ---------- | ------ | --------- | ------------ | -------- |
| state | free_value, animation | String | State of the state machine | * |
| values | Array of 0 to 255 | Array of Integer |Max. length 4 | * |
| animation_name | ramp_up_down | String | | |
| options |  | JSON | Advanced options hash | |
| initial_mode | true, false | Boolean |Load on startup | |

### POST /say

Synthesize text to speech.

| Parameters | Values | Data Type | Description  | Required |
| ---------- | ------ | --------- | ------------ | -------- |
| value | Hello World | String | TTS String | * |

### POST /power

Control the high-voltage power supply.

| Parameters | Values | Data Type | Description  | Required |
| ---------- | ------ | --------- | ------------ | -------- |
| value | 0 or 1 | String | Turn on or off the high voltage power supply | * |

### POST /scheduler

## GET Requests

Get requests to get information about the nixon-pi.

### GET /info/:target.:format

Get information about the current state of a specified state machine.

| Parameters | Values |
| ---------- | ------ |
| :target | tubes, bars, lamps, power |
| :format | html, json |

**Example:**

	wget http://localhost:3000/info/tubes.json

Returns:

	{
	    "info": {
	        "animation_name": "single_fly_in",
	        "options": {},
	        "last_value": "214740",
	        "last_state": "animation",
	        "state": "time",
	        "last_time": "2013-01-16T21:47:40+01:00"
	    },
	    "message": "tubes set to",
	    "success": true
	}

### GET /info.:format

Get hardware information about the Raspberry-Pi I/O expansion card.
Information include:

* RTC - Real time clock
* IO  - GPIO driver
* ADC - Analog digital converter driver
* PWM - Pulse width modulation driver


| Parameters | Values |
| ---------- | ------ |
| :format | html, json |

**Example:**

	wget http://localhost:3000/info.json

Returns:

	{
	    "info": {
	        "rtc": "1",
	        "io": "1",
	        "adc": "1",
	        "pwm": "1"
	    },
	    "message": "Hardware information",
	    "success": true
	}

### GET /command/:target.:format

Get a json containing all available commands for a specified target.

| Parameters | Values |
| ---------- | ------ |
| :target | tubes, bars, lamps, power, say |
| :format | html, json |

**Example:**

	curl http://localhost:3000/command/tubes.json

Returns:

	{
	    "state": null,
	    "value": null,
	    "time_format": null,
	    "animation_name": null,
	    "options": null,
	    "initial_mode": null
	}

### GET /commands.:format

Get a json of all available commands.

| Parameters | Values |
| ---------- | ------ |
| :format | html, json |


Return a json of available commands and parameters.

**Example:**

	curl http://localhost:3000/commands.json

Returns:

	{
	    "tubes": {
	        "state": null,
	        "value": null,
	        "time_format": null,
	        "animation_name": null,
	        "options": null,
	        "time": null,
	        "initial_mode": null
	    },
	    "bars": {
	        "state": null,
	        "values": null,
	        "animation_name": null,
	        "options": null,
	        "time": null,
	        "initial_mode": null
	    },
	    "lamps": {
	        "state": null,
	        "values": null,
	        "animation_name": null,
	        "options": null,
	        "time": null,
	        "initial_mode": null
	    },
	    "power": {
	        "value": null,
	        "time": null
	    },
	    "say": {
	        "value": null,
	        "time": null
	    },
	    "message": "Available commands",
	    "success": true
	}

## DELETE Requests

Delete saved values.

### DELETE /schedule/:id

Delete a saved schedule.

| Parameters | Values |
| ---------- | ------ |
| :id | id of an existing schedule |


**Example**

    curl -X DELETE http://localhost/schedule/1

Returns:

	{
		"success":true,
		"message":"Schedule deleted"
	}
	


