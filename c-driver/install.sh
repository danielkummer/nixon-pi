#!/bin/bash


#
# Build the executables if non-existent
#

[ -f abiocardtime ] || make -f abiocardtime.mk
[ -f abiocardserver ] || make -f abiocardserver.mk


#
# Root
#

sudo ./install_root.sh

