# API

All api requests can be made as either .html or .json.
JSONP is returned to support 3rd party apps.

## Port

The web server port can be configured in the nixonpi-settings.yml file inside your home directory.
	
	prodiction:
	  web_server:
	    port: 3000
	

## POST Requests

### POST /tubes/(:id)

| Parameters | Values | Description  |
| ---------- | ------ | ------------ |
| state | time, free_value, animation, countdown, run_test |     |
| value | 0-9 or _ | Max. length 12 |
| time_format | ex: "%H%M%S" | only in state "time" |
| animation_name | single_fly_in, count_from_to, switch_numbers |  |
| options |  | Advanced options hash |
| initial_mode | true, false | Load on startup |

### POST /lamps/(:id)

| Parameters | Values | Description  |
| ---------- | ------ | ------------ |
| state | free_value, animation, run_test |     |
| values | Array of 0 or 1 | Max. length 5 |
| animation_name | single_fly_in, count_from_to, switch_numbers |  |
| options |  | Advanced options hash |
| initial_mode | true, false | Load on startup |

### POST /bars/:id

| Parameters | Values | Description  |
| ---------- | ------ | ------------ |
| state | free_value, animation |     |
| values | Array of 0 to 255 | Max. length 4 |
| animation_name | ramp_up_down |  |
| options |  | Advanced options hash |
| initial_mode | true, false | Load on startup |

### POST /say

| Parameters | Values | Description  |
| ---------- | ------ | ------------ |
| value | Hello World | Max. length 4 |

### POST /power

| Parameters | Values | Description  |
| ---------- | ------ | ------------ |
| value | 0 or 1 | Turn on or off the high voltage power supply |

### POST /scheduler

## GET Requests

### GET /info/:state_machine.:format

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

### GET /command/:name.:format

| Parameters | Values | 
| ---------- | ------ | 
| :name | tubes, bars, lamps, power, say | 
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

### DELETE /schedule/:id

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
