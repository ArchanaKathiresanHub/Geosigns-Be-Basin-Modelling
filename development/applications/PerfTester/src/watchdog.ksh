#!/bin/bash
#
# This scripts monitors PerfTester jobs to see if trhey are still
# runing. If not then gempis is used to terminate the job.
#

echo "$0: watch dog process for PerfTester......."
echo ""

cd /glb/home/nlgla8/SWEast/PerfTester/Work

while true; do

    # look for files in the Work directory that have not changed status in 24 hours
    for logfile in `find . -name '*.log' -cmin +1440 `; do

	echo "Log file $logfile has not responded in 24 hours."
	jobid=`cat $logfile | grep 'Job <' | awk '{ print $2 }' | sed 's/<//' | sed 's/>//'`
	if [ -n "$jobid" ]; then
	    echo "    Terminating job $jobid."
	    gempis res Darwin kill $jobid
	fi
    done

    sleep 120
done
