#! /bin/bash
#
#
# This script can be used to create .cbm files from old log data. 
#  !!! Some assembly required !!!
#

export JOURNAL_FILE=`pwd`/Old-Temp.journal 
export OUTPUT_DIR=`pwd`  
export TEST="Old-NEMed"
export RESOURCE=MIB

for file in `ls $TEST/$search*` ; do

	export CORES=`grep rank $file | wc -l | awk '{ print $1 }'`

	project=`echo $file | awk -F'/' '{ print $2 }' | awk -F'-' '{ print $1 }'`
	export COMMAND="fc -project $project.project3d -temperature"
	export TRIAL="fc-temp-$project"

	echo "	cbm_perf evaluate $file"
	cbm_perf evaluate $file

done