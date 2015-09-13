#!/bin/bash
#
# Point a Desktop launcher at this shell script rather than gHermes.
# It will set the current diectory correctly.
#
# You can also use icon.png as the icon for the launcher.
#
cd `dirname $0`
pwd > gHermes.log 2>&1
./gHermes --metis --interface eth1 >> gHermes.log 2>&1
