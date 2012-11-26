# Nixon-&PI; - The Raspberry Pi based Nixie Display

## The components

The Nixon-&PI; Display consists of Nixie Tubes provided by [Ogilumen](http://www.ogilumen.com/), driven by a Raspberry Pi Model B with an [Axiris](http://www.axiris.be/) I/O expansion card.

### 1x Raspberry Pi Model B

<img src="http://www.raspberrypi.org/wp-content/uploads/2012/04/Raspi_Iso_Blue.png" width="200" height="150">
---
The [Raspberry Pi](http://www.raspberrypi.org/) is ideal for the project as it's a fully flegded mini computer which is capable of running all the things we need (Ruby, a web server, ...).

### 1x [Axiris](http://www.axiris.be/) I/O Card for Raspberry Pi

<img src="http://www.axiris.be/en/images/stories/bcm2835/bcm2835_013_500.jpg" width="200" height="150">
---
Because the Raspberry Pi only has limited IO capabilites the [Axisis I/O card](http://www.axiris.be/en/index.php?option=com_content&view=article&id=51:i2c-io-card-for-use-with-raspberry-pi-computer&catid=14:io-cards&Itemid=34) is the perfect addition to fully satisfy our needs. It adds:

* Very high accuracy real-time clock based on the PCF2129A with CR2032 backup battery.
* 8 quasi bidirectional I/O lines (PCF8574), 5V levels.
* 8 12-bit ADC inputs (MAX11614EEE+), 0 - 4.096V range.
* Model B only - 16 output channels providing 12-bit pulse-width modulation (PWM) output channels at about 40 to 1000 Hz with LED drive capability, 10 mA source, 25 mA sink, 5 V levels.

### 6x Nixie Duo Kits

<img src="http://www.ogilumen.com/images/product_pics/n2xdp1.jpg" width="200" height="150">
---

[Ogilumen](http://www.ogilumen.com/) sells beautiful, compact and easy to assemble nixie tube kits containing two IN-12A tubes each.

### 4x IN-13 Bar Graphs


<img src="http://www.ogilumen.com/images/product_pics/IN-13a.jpg" width="200" height="150">
---

The IN-13 Neon Bar Graphs sold by Ogilumen are a beautiful way to display a variable length glowing bar.


### 5x IN-1 Neon


<img src="http://www.ogilumen.com/images/product_pics/n1a.jpg" width="200" height="150">
---
The IN-1 Neon Lamps are simply a nice LED-alternative and they have a nice warm orange glow...


## 1x 50 mA Nixie Tube Power Supply

<img src="http://www.ogilumen.com/images/product_pics/smps1.jpg" width="200" height="150">
---

Lastly we need a power supply to drive all our high-voltage Nixie goodness. Luckly ogilumen provides us with a readily assembled power supply capable of providing all the power we need...



## Circuit Diagrams

TODO

## Case Designs

TODO


## The Service

A telnet based client to drive nixie tubes via rest interface

### Manage the daemon process

Manage the daemon with

    ruby app/nixieserver_control.rb start
    ruby app/nixieserver_control.rb restart
    ruby app/nixieserver_control.rb stop

### Controlling

The daemon is controlled via sinatra web application (REST-full), see the example page at [YOUR-IP]:9999

## The Gem

The Nixon-&PI; Service is packed as a standalone gem which can be run as a daemon.

### Build

Build with

    gem build Nixon-&PI;-service.gemspec


### Install

    $ gem install nixie-berry-service



### State machine

The service uses a state machine to handle local executions.
Generate state machine diagrams with the following command:

    rake state_machine:draw FILE=./lib/nixon-pi/state_machines/tube_state_machine.rb CLASS=NixonPi;::TubeHandlerStateMachine

# Copyright


