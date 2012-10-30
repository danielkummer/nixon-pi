# Nixieberry::Service

TODO: Write a gem description

## Build

Build with

    gem build nixieberry-service.gemspec

## Installation

Add this line to your application's Gemfile:

    gem 'nixieberry-service'

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install nixieberry-service

## Usage

TODO: Write usage instructions here



= nixie-berry service

A telnet client to drive nixie tubes

== Managing daemon

Manage the daemon with

    ruby app/nixieserver_control.rb start
    ruby app/nixieserver_control.rb restart
    ruby app/nixieserver_control.rb stop

== Controlling

The daemon is controlled via redis. The following keys are used:

* mode - operation mode, accepts display_free_value display_time
* free_value - tube values if mode is display_free_value
* time_format - output format "%H%M%S", see http://www.ruby-doc.org/core-1.9.3/Time.html

== State machine

The service uses a state machine to handle local executions.
Generate a state machine diagram with

    rake state_machine:draw FILE=./lib/state_machines/nixie_state_machine.rb CLASS=NixieBerry::NixieStateMachine

== Copyright

Copyright (c) 2012 Daniel Kummer. See LICENSE.txt for
further details.

