#!/bin/bash
#

if [ -z "$3" ] ;then
    cat <<EOF 


This program runs elfen in an external coupling mode. 

Usage $0 <numprocs> <mechanical_project> <seepage_project> [<delay>]

The file names are without the .dat. Examples:
   mechanical file = pl_cs01n01
   seepage file    = pl_csp01n01,
The delay is how long it takes for a job submitted to the cluster 
to run. The default is 20 seconds.

The output from each application is also stored in a .log file.


EOF
    exit 1
fi

numprocs=$1
mechanical_file=$2
mechanical_sync=STRUCT.MAL

seepage_file=$3
seepage_sync=SEEPAGE.MAL

delay=$4
[ -z "$delay" ] && delay=20

if [ ! -f $seepage_file.dat ]; then
    echo "Seepage file $seepage_file.dat not found."
    exit 1
fi
if [ ! -f $mechanical_file.dat ]; then
    echo "Mechanical file $mechanical_file.dat not found."
    exit 1
fi
# create dummy data files
touch $seepage_sync $mechanical_sync

echo "Submitting Mechanical file $mechanical_file to cluster."
launch $numprocs $mechanical_file 2>&1 | tee $mechanical_file.log &

echo "Waiting $delay seconds."
sleep $delay

echo "Running Seepage file $seepage_file locally"
elfendyn $seepage_file 2>&1 | tee $seepage_file.log

# reset terminal
stty sane

# End Script
