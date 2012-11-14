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


= nixie-berry service

A telnet based client to drive nixie tubes via rest interface

== Managing daemon

Manage the daemon with

    ruby app/nixieserver_control.rb start
    ruby app/nixieserver_control.rb restart
    ruby app/nixieserver_control.rb stop

== Controlling

The daemon is controlled via sinatra web application, see the example page at [YOURIP]:9999

== State machine

The service uses a state machine to handle local executions.
Generate a state machine diagram with for example:

    rake state_machine:draw FILE=./lib/nixieberry/handlers/tube_handler_state_machine.rb CLASS=NixieBerry::TubeHandlerStateMachine --trace

== Copyright

Copyright (c) 2012 Daniel Kummer. See LICENSE.txt for
further details.

