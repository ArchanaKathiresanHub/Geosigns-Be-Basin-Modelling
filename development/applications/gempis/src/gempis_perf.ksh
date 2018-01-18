#
# gempis_perf - GEneric MPI Submission performance tool
#
# this file exists only on Global Linux releases
#if [ -f /etc/shell-release ]; then
# define interrupt handler
#glversion=`cat /etc/shell-release | awk '{ print $4 }'`
#if [ "$glversion" = gl23 ]; then
# defining TERM in GL 2.3 causes the script to abort
#
#############################################################################

# check that this routine has been called in the correct wat
if [ -z "$GEMPIS_USERNAME" ]; then
    echo "Error! Invalid command format, please use 

    gempis perf

instead.
"
    exit 1
fi


# name use by CSCE to locate this program
export GEMPIS_PROGNAME="gempis_perf"
verbose "$GEMPIS_PROGNAME [$*]."

# define global variables
benchfile="$HOME/.gempis/benchmarks"
modelsize=""

datetime=""
gempisid=""
jobid=""
userid=""
computer=""
platform=""
osversion=""
cwd=""
disk=""
appcommand=""
application=""
appversion=""
options=""
modelname=""
modelnodes=""
recomendedmemory=""
resource=""
numprocs=""
result=""
duration=""
outputsize=""
memoryused=""
swapused=""
numnodes=""


############################################################################
#
# tell user how I work and exit
#
# This one is defined but not implemented
usage()
{
    # terminate application 
    echo "Invalid command line. The options are:

    gempis perf help
                <resource> <#processors> [<cauldron option>...]
                list       <#processors> [<cauldron option>...]
"

exit 1

# This stuff is no longer active
    based=$IBS_NAMES/benchmarks
    list=`ls $based`
    verbose "Parsing list [$list]."
    for file in $list; do
	model=`echo $file | awk -F'.' '{ print $1 }'`
	list=`tar tvfz $based/$file | grep .project3d | awk '{ print $6 }' | awk -F'_' '{ print $2 }' | awk -F'.' '{ print $1 }'`
	nlist=`echo $list`
	echo "$model: $nlist"
    done
    echo ""
    exit 1
}


############################################################
#
# get information about the job from LSF
#
collect_lsfinfo()
{
    # load information about the resource
    load_resources
    define_resource $resource

    # a short wait for times to settle down
    delay=30
    echo "Waiting $delay seconds before collecting LSF information"
    sleep $delay

    verbose "Collecting LSF information from job [$jobid]."
    lsf_file=lsf.info
    if [ -n "$GEMPIS_HEADNODE_LIST" ]; then

        # find a valid headnode
	find_headnode

	# send command to headnode
	ssh -q -t -o 'BatchMode=yes' -o 'StrictHostKeyChecking=no' $GEMPIS_HEADNODE ". /etc/profile; bjobs -l $jobid" | tee lsf.info
    else
	# run command locally
	bjobs -ld $jobid | tee $lsf_file
    fi

    # the job file contains some interesting information, tha may or may not be there
    # ex. "Tue Apr 24 15:52:17: Resource usage collected.
    #                           MEM: 5 Mbytes;  SWAP: 91 Mbytes;  NTHREAD: 4"
    memoryused=`cat $lsf_file | grep "MEM:" | sed 's/;//g' | awk '{ printf("%d%s", $2, $3) }'` 
    swapused=`cat $lsf_file | grep "SWAP:" | sed 's/;//g' | awk '{ printf("%d%s", $5, $6) }'`
    # ex. "Tue Apr 24 15:48:49: Done successfully. The CPU time used is 2.1 seconds."
    # ex. "Wed Apr 25 14:10:05: Exited with exit code 127. The CPU time used is 0.6 second"
    #     "                     s."
    resline=`cat $lsf_file | grep "CPU time used"`
    successful=`echo "$resline" | grep "successfully"`
    if [ -n "$successful" ]; then
	result=0
	cputime=`echo "$resline" | awk '{ printf("%s", $12) }'` 
    else
	result=`echo "$resline" | awk '{ printf("%d", $9) }'`
	cputime=`echo "$resline" | awk '{ printf("%s", $15) }'` 
    fi
    verbose " Result = [$result], CPU time is [$cputime]."

    # get the number of nodes the job was run on
    verbose "Extracting node information."
    host_line=`cat $lsf_file | cut -c22-79 | while read cmd line; do
	    if [ "$cmd" = "Started" ]; then
		collect=yes
	    elif [ -z "$cmd" -o "$cmd" = "Done" -o "$cmd" = "Exited" ]; then
		collect=""
	    fi
	    [ -n "$collect" ] && echo -e "$cmd $line\c"
	done`
    if [ -n "$host_line" ]; then
	numnodes=`echo "$host_line" | awk -F'<' '{ print NF-1 }'`
	verbose "This job ran on $numnodes nodes"
    fi
}


############################################################
#
#
#
do_appcommand()
{
    # set evvironment variable to indicate benchmark
    export GEMPIS_PROJECTNAME="Benchmark"

    # define the command 
    verbose "The command to be run is [$appcommand]."

    # run the command
    echo ""
    echo "Executing command:"
    echo "    gempis $version $resource $numprocs $appcommand"
    gempis $version $resource $numprocs $appcommand
    result=$?

    if [ "$result" = 0 ]; then
	echo "The test has SUCCEEDED."
    else
	echo "The test has FAILED, error ($result)."
    fi
}


############################################################################
#
#
#
do_test()
{
    # unpack tar file
    verbose "Unpacking test file [$tarfile]."
    tar xfz $tarfile

    verbose "Moving into directory [$modelname]."
    cd $modelname
    if [ $? != 0 ]; then
	echo "Could not move into the test directory [$modelname], terminating."
	echo ""
	usage
	exit 1
    fi

    # define the model file name
    if [ ! -f $modelfile ]; then
	echo "Error! model file [$modelfile] not found."
	echo ""
	usage
	exit 1
    fi

    # if we are running under CSCE define the version
    if [ -z "$GEMPIS_NOT_CSCE" ]; then
	appversion="-${IBS_VERSION}"
	verbose "Version set to [$version]."
    fi

    appcommand="fastcauldron $appversion -project $modelfile $options"

    #
    # this is where the command is executed
    #
    do_appcommand | tee run.log

    #
    # Each line in the file run.log is terminated with a \r.
    # Filter this out when reading from the file.
    #

    # set time stamp and print time
    end=`date '+%s'`
    let 'dif = end - begin'
    mytime=`date -d "1970-01-01 00:00 $dif seconds" +"%H:%M:%S"`

    # get execution time
    apptime=`grep " Calculation:" run.log | tr '\r' ' '`
    if [ -z "$apptime" ]; then
	echo "No time duration found in log file, error..."
	result=1
	outputsize=0
	duration=0
    else
        # get size of output directory
	outputsize=`du --si ${modelname}_*_CauldronOutputDir | awk '{ print $1 }'`

        # get the duration
	verbose "Extracting time from [$apptime]."
	duration=`echo "$apptime" | awk '{ 
if ( $1 == "P/T" )
    printf( "%0d:%0d:%0d\n", $4, $6, $8);
else 
    printf( "%0d:%0d:%0d\n", $3, $5, $7);
}'`
    fi

    # if app version not already known try to find it in the run log
    if [ -z "$appversion" ]; then
	appversion=`cat run.log | grep "\'Launching \'fastcauldron\' version: " | tail -1 | tr '\r' ' ' | awk '{ print $4 }'`
    fi

    # get the amounty of memory required
    line=`fastreq $version -project $modelfile | grep "Gempis_DATA MEMORY"`
    verbose "Checking line [$line] for memory info."
    recomendedmemory=`echo "$line" | awk '{ 
if ( $4 >= 1024 )
    printf("%.2f GB\n", $4/1024);
else
    printf("%d MB\n", $4);
}'`

    gempisid=`cat run.log | grep "JOBID" | tail -1 | tr '\r' ' ' | awk '{ printf("%s",$2) }'`

    # currently only defined for LSF
    jobid=`cat run.log | grep "^Job <" | tail -1 | tr '\r' ' ' | awk -F'<' '{ print $2 }' | awk -F'>' '{ print $1 }'`
#    [ -n "$jobid" ] && collect_lsfinfo

    # display information
    echo "GEMPIS Performance Benchmark"
    echo ""
    echo " 1 - Date:              $datetime"
    echo " 2 - Gempis Id:         $gempisid"
    echo " 3 - Job Id:            $jobid"
    echo " 4 - User Id:           $userid"
    echo " 5 - Computer:          $computer"
    echo " 6 - Platform:          $platform" 
    echo " 7 - O/S version:       $osversion"
    echo " 8 - Directory:         $cwd"    
    echo " 9 - Disk:              $disk"    
    echo "10 - Command:           $appcommand" 
    echo "11 - Application:       $application"
    echo "12 - App version:       $appversion"
    echo "13 - Options:           $options"
    echo "14 - Model name:        $modelname" 
    echo "15 - Model grid points: $modelnodes"
    echo "16 - Recomended memory: $recomendedmemory"
    echo "17 - Resource:          $resource"
    echo "18 - Number of cores:   $numprocs"
    echo "19 - Number of nodes:   $numnodes"
    echo "20 - Result:            $result"
    echo "21 - Duration:          $duration"
    echo "22 - OutputSize:        $outputsize"
    echo "23 - Memory used:       $memoryused"
    echo "24 - Swap used:         $swapused"

    # Benchmark file format:
    #  Date=<date>|
    #  GempisId=<gempis id>|
    #  JobId=<LSF or other job id>|
    #  UserId=<userid>|
    #  Computer=<computer name>|
    #  Platform=<platform>|
    #  OSVersion=<osversion>|
    #  Directory=<cwd>|
    #  Disk:<disk device>|
    #  Command=<command line>|
    #  Application=<application name>|
    #  AppVersion=<appversion>|
    #  Options=<options>|
    #  ModelName=<model name>|
    #  ModelNodes=<model computational nodes>|
    #  RecomendedMemory=<recomended memory>|
    #  Resource=<resource name>|
    #  NumberProcessors=<number of cores>|
    #  NumberNodes=<number of computers>|
    #  Result=<result>|
    #  Duration=<duration>|
    #  OutputSize=<size of output>|
    #  MemoryUsed:<total memory used>|
    #  SwapUsed:<total swap used>|

    # add line to benchmarks file
    verbose "Adding entry to benchmark file."
    echo "Date=$datetime|GempisId=$gempisid|JobId=$jobid|UserId=$userid|Computer=$computer|Platform=$platform|OSVersion=$osversion|Directory=$cwd|Disk=$disk|Command=$appcommand|Application=fastcauldron|AppVersion=$appversion|Options=$options|ModelName=$modelname|ModelNodes=$modelnodes|Recomendedmemory=$recomendedmemory|Resource=$resource|NumberProcessors=$numprocs|NumberNodes=$numnodes|Result=$result|Duration=$duration|OutputSize=$outputsize|MemoryUsed=$memoryused|SwapUsed=$swapused|" >> $benchfile

    # clean up my mess
    if [ -z "$GEMPIS_DEBUG" ]; then
	cd ..
	rm -rf $modelname
    fi
}


############################################################
#
# main
#

# define the system and user environment settings
load_definitions

# if no parameters then display usage message
[ $# -lt 1 ] && usage

# process some of the simple commands
if [ "$1" = "help" ]; then
    show_help
    exit 0
fi

# say hello
echo "Gempis performance test"
echo ""

# get parameters
resource=$1
numprocs=$2
shift
shift
echo "Resource = $resource, Number of processors = $numprocs."

# decide on the model file and model size to use for the job
modelname=Acquifer
modelsize=00M03

# define default option if not used
if [ -z "$*" ]; then
    options="-itcoupled"
else
    options="$*"
fi
verbose "Options  [$options]."

# define the format to use for date
dateformat='+%Y.%m.%d-%H:%M:%S'
datestamp="`date $dateformat`"

# copy model stuff into test directory
srcdir=$IBS_NAMES/benchmarks
if [ ! -d $srcdir ]; then
    echo "Could not find test source directory [$srcdir]."
    echo ""
    echo "Terminating..."
    echo ""
    exit 1
fi
tarfile=$srcdir/$modelname.tar.gz
if [ ! -f $tarfile ]; then
    echo "Could not find test data file [$tarfile]."
    echo ""
    usage
    exit 1
fi

# set begin ts
application=fastcauldron
modelfile=${modelname}_${modelsize}.project3d
datetime=`date $dateformat`
begin=`date '+%s'`
cwd=`pwd`
platform=`getos2`
osversion=`uname -r`
computer=`hostname -s`
userid=`whoami`
disk=`df . | tail -n +2 | head -1` 
[ -z "$disk" ] && disk=`df . | tail -n +2 | awk '{ print $1 }'`
# adjust model size to be the same format as output
modelnodes=`echo $modelsize | awk -F'M' '{ printf("%d\n", ($1 * 1048576) + ($2 * 10240) )}'`

# do the test
[ "$resource" != "list" ] && do_test

# compare execution times with previous values
echo "Previous ${modelname}_${modelsize} runs with $numprocs processors were:" 
echo ""
printf "%20s %10s %8s %12s\n" Date Resource Result Duration
echo "--------------------------------------------------------------"
if [ -f $benchfile ]; then

    verbose "Scanning for |NumberProcessors=$numprocs| and |Options=$options|"

    firstline=yes
    cat $benchfile | grep "|NumberProcessors=$numprocs|" | grep "${modelname}_${modelsize}.project3d" | grep "|Options=$options|" | while read line; do
	
	dt=`echo "$line" | awk -F'|' '{ print $1 }' | awk -F'=' '{ print $2 }'`
	resource=`echo "$line" | awk -F'|' '{ print $17 }' | awk -F'=' '{ print $2 }'`
	result=`echo "$line" | awk -F'|' '{ print $20 }' | awk -F'=' '{ print $2 }'`
	dur=`echo "$line" | awk -F'|' '{ print $21 }' | awk -F'=' '{ print $2 }'`
	
	printf "%20s %10s %8s %12s\n" $dt $resource $result $dur
    done
else
    echo "Benchmark file [$benchfile] does not exist."
fi

# End Script
