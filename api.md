

# API

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

### GET /info/:state_machine.:format

Get information about the current state of a specified state machine.

| Parameters | Values |
| ---------- | ------ |
| :state_machine | tubes, bars, lamps |
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
