#!/bin/bash


#
# Build the executable if non-existent
#

[ -f abiocardtime ] || make -f abiocardtime.mk


#
# Root
#

sudo ./install_abiortc_rpi_root.sh

