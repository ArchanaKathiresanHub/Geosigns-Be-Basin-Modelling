#!/bin/bash
#
# This scripts contains local definitions for PerfTester
#
# Syntax:
# . ./setup.ksh [standalone]
#
# standalone - setup the test environment for standalone use.
#

echo "$0: setting up PerfTester environment......."
echo ""

export PERFTESTER_HOME=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/PerfTester

export PATH=${PERFTESTER_HOME}/bin:$PATH

if [ "$1" = "standalone" ]; then

    # if your here the setup Perftester for standalone use
cat <<EOF

This script sets up a standalone SSS environment

EOF

    export LM_LICENSE_FILE=3000@AMSDC1-S-768.europe.shell.com:3000@CBJ-S-459.asia-pac.shell.com:3000@HOUIC-S-438.americas.shell.com:/apps/3rdparty/etc/licenses/license.dat:/var/flexlm/license.dat
    export LM_PROJECT=SIEP_BV

    export SIEPRTS_LICENSE_FILE=3000@AMSDC1-S-768.europe.shell.com:3000@CBJ-S-459.asia-pac.shell.com:3000@HOUIC-S-438.americas.shell.com

    export PATH=${PERFTESTER_HOME}/sssdev/share:$PATH
fi


