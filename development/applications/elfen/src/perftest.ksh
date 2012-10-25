#!/bin/bash
#
# runs through a series of test varying the number of processors
#
# log file is saved for use later.
#
# Dependencies:
# (c)setup - The elfen setup or csetup script defines the environment used here.
# launch - launches elfen jobs


logdir=${ELFEN_DIR}/perf/logs
resource=Elfen
date=`date '+%Y.%m.%d-%T'`
# cpu_list, take off one for the number of slaves
#cpu_list="3 5 7 9 11 13 16" 
cpu_list="16 13 11 9 7 5 3" 
project_list="vpsx039dp vpsx039g"

# run loop
echo "Moving into projects directory."
cd ${ELFEN_DIR}/perf/projects

for project in $project_list; do

    echo "Testing performance of project $project."
    for cpu in $cpu_list; do

	logfile=$logdir/${project}_${resource}_${cpu}_${date}.log

	echo "Unpacking project file."
	tar xvfz ${project}.tar.gz
	cd ${project}.elf

	echo "launching on $cpu CPUs, logfile is [$logfile]"
	launch $cpu $resource | tee $logfile

	echo "Cleaning up..."
	cd ..
	rm -rf ${project}.elf

    done

done

# End script
