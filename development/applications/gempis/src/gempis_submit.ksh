#
# GEMPIS - GEneric MPIrun Submission
#
# Purpose: a script to generically submit MPI jobs
#
# Description: MPI programs using ssh to spawn jobs have the problem that 
# the users environment variables are not available to executables spawned 
# by ssh. This is true even when the users  ~/.ssh/rc script has been defined 
# and executed. 
# The purpose of this script is to create another script containg the users 
# environment as it was when it was called. This script is then submitted to 
# MPI with the mpirun command.
#
# Syntax: 
# gempis <resource> <np> <executable> <executable_options> 
#
# executable        - the mpich executable to be run. If a path is defined 
#                     in the executable name, it is assumed to be a local 
#                     executable. Otherwise it assumes that the executable 
#                     is under /apps/sss(dev) (useing PATH to find it).
# executable_option - program arguments
#
# External Environment Variables:
#   PATH             - 
#   EXEDIR           - directory defining location of binaries
#   IBS_NAMES        - (required) root directory for IBS
#   IBS_VERSION      - (under CSCE) version of application to run
#   LM_LICENSE_FILE  - location of license files (required by most 
#                      applications)
#   machine          - name defining operating system
#
#   GEMPIS_VERBOSE - when defined generates additional messages
#   GEMPIS_DEBUG   - when defined does not remopve data files
#   GEMPIS_HOSTNAME - defined by gempis.ini as the computer calling 
#                    /apps/sss[dev]/share/gempis
#   GEMPIS_SSH - the ssh command path. This value is evaluated every time the
#                gempis command is called.
#
# Dependencies:
# - Linux, Solaris, Irix - only tested on these platforms
# - mpirun - will be found using the PATH. 
# - ssh    - it is assumed that the authorization details have been taken 
#            care of 
#          (ie. RSA/DSA keys have been created for the servers used).
#
# Notes:
# 1. This script is used by each remote ssh process to define the environment 
# variables.
# 2. Will only work when the current working directory is accessable to all 
# of the machines where it will be run.
# 1. This script creates a temporary file <executable>-<XXXXXX>/ 
#     gempis_appwrapper 
# (where XXXXXX is the job id) in the current working directory. 
#############################################################################


#############################################################################
#
# Setup CSCE if not defined

# if we are not running under CSCE then setup our own environment.
if [ -z "$EXEDIR" -o -n "$GEMPIS_NOT_CSCE" ]; then
    echo "MeSsAgE INFO Gempis is not running under CSCE."

    # set executable directory to that used by this script
    export EXEDIR=`dirname $0`

    # this variable will be used through the rest of the script to determine 
    # whether we are running under CSCE (/apps/sss(dev)) or not
    export GEMPIS_NOT_CSCE="yes"
fi
# This definition is used througout gempis to locate gempis executables
export GEMPIS_EXEDIR=$EXEDIR

# Set the run time estimate before it is too late
for arg in $* ; do
   case "$arg" in
   *:*) export GEMPIS_TIME="$arg"; break;;
   esac
done

#
# define gempis utilities
#
. $GEMPIS_EXEDIR/gempis_utils

############################################################################
#
# interrupt handler subroutine, when INT has been pressed
#
trap_handler()
{
    # terminate application
    terminate ABORT "Interrupt detected by Gempis."
}

pre_trap_handler()
{
    # terminate application
    echo "Pre-interrupt detected by Gempis."
    exit 2
}

############################################################################
#
# determine the command
verbose "Checking parameter [$1] for command."
case $1 in
adm)
	shift
	. $GEMPIS_EXEDIR/gempis_adm $*
	exit $?
	;;
cfg)
	shift
	. $GEMPIS_EXEDIR/gempis_cfg $*
	exit $?
	;;
mon)
	shift
	. $GEMPIS_EXEDIR/gempis_mon $*
	exit $?
	;;
perf)
	shift
	. $GEMPIS_EXEDIR/gempis_perf $*
	exit $?
	;;
res)
	shift
	. $GEMPIS_EXEDIR/gempis_resource $*
	exit $?
	;;
stat)
	shift
	. $GEMPIS_EXEDIR/gempis_stat $*
	exit $?
	;;
esac
if [ "$1" = "resources" ]; then
    . $GEMPIS_EXEDIR/gempis_cfg resource
    exit 0
fi


#
# if here then the command was a straight gempis
#

# name use by CSCE to locate this program
export GEMPIS_PROGNAME=gempis
verbose "$GEMPIS_PROGNAME [$*]."

############################################################################
#
# tell user how I work and exit
#
usage()
{
    # terminate application 
    terminate ERROR "Invalid command line. The options are:

    gempis help
           resources
           <resource> [loop]
           <resource> <numcpus> <executable> [<arguments> ...]

    gempis adm help
               setup
               keygen
               keyrestore
               cleanup
               email <filename> [user@<destination>]

    gempis cfg help
               computer
               defs
               jobtype [<jobtype>]
               mpiref [<mpirfe>]
               resource [<resource>]
               export 
               import 

    gempis mon help
               <resource> <jobid>

    gempis perf help
                <resource> <#processors> [<option>...]
                list       <#processors> [<option>...]

    gempis res help
               <resource>
               <resource> kill <jobId>
               <resource> killall
               <resource> test
               <resource> loop [<delay>]
               <resource> daemon

    gempis stat help
                <resource> collect
                <resource> resource [YYYY-MM]
                user [YYYY-MM]
"
    exit 0
}


############################################################################
#
# builds a script defining all current environment variables and 
#          call application
#
# Description: Some programs using ssh to spawn commands on remote computers 
# have the problem that the user environment is not available. This is true 
# even when a users ~/.ssh/rc script has been defined. 
#
# Syntax: build_appwrapper <wrapper_name> <executable> [<arguments> ... ]
#
# myfile - name of script file created.
# exec   - the executable to be run, assumes that executable exists.
#          If a path has been given then the executable is local,
#          otherwise it is assumed to be a CSCE startup script.
# args   - program arguments
#
build_appwrapper()
{
    myfile=$1
    shift
    verbose "Building application wrapper script [`basename $myfile`]."

    exec=$1
    shift
    args="$*"

    #
    # determine if a path has been given in the command in the following way:
    # - if a path has been given in the command then the command is not to be
    #   run under CSCE so no version number
    # - otherwise the application should be run under CSCE, so include version 
    #   number
    #

    # define version here so it is seen below
    MyVersion=""
    testpath="`echo $exec | awk -F'/' '{ print $1 }'`"
    if [ "$testpath" = "$exec" -a -n "$IBS_VERSION" ]; then
	MyVersion="-${IBS_VERSION}"
	verbose "The executable [$exec] is assumed to be under CSCE, version [$MyVersion]."
    else
	verbose "The executable [$exec] is local."
    fi
    #
    verbose "The appwrapper command is [$exec $MyVersion $args]."

    # reset file and add a bash shell header
    echo "#!/bin/bash" > $myfile
    [ $? -ne 0 ] && terminate ERROR "Cannot create application wrapper script 
    $myfile. 
    Permission denied."

    # if defined emable debugging of the appwrapper script
    if [ -n "$GEMPIS_DEBUGAPPWRAPPER" ]; then
	verbose "Setting appwrapper verbose."
	echo "set -x" >> $myfile
    fi

    # say where this script came from 
    echo "#" >> $myfile
    echo "#  This temporary script was generated by $0" >> $myfile
    echo "#   and can be removed if found." >> $myfile
    echo "#" >> $myfile
    echo "#  Created on `get_datetime`" >> $myfile
    echo "#" >> $myfile
    echo "#  Restoring user environment." >> $myfile

    # write user environment to file
    dump_user_environment >> $myfile

    # setup environment for this platform
    echo "#" >> $myfile
    echo ". $GEMPIS_EXEDIR/gempis_utils" >>$myfile
    echo 'result=$?' >> $myfile
    echo 'if [ "$result" != 0 ]; then ' >>$myfile
    echo '    echo "AppWrap.`hostname`($$): Failed to run $GEMPIS_EXEDIR/gempis_utils"' >>$myfile
    echo 'fi' >>$myfile

    # echo '. /apps/3rdparty/intel/ics2011/impi/4.0.1.007/bin/mpivars.sh' >> $myfile
    # replaced by
    echo ". $INTELMPI_HOME/bin/mpivars.sh" >> $myfile

    # move into my current directory
    echo "#" >> $myfile
    echo "cd $GEMPIS_CWD" >> $myfile
    echo 'result=$?' >> $myfile
    echo 'if [ "$result" != 0 ]; then' >>$myfile
    echo '    echo "AppWrap.`hostname`($$): Move to directory [$GEMPIS_CWD] failed."' >>$myfile
    echo 'else' >>$myfile
    echo '    verbose "AppWrap.`hostname`($$): Moved into [$GEMPIS_CWD]."' >> $myfile
    echo 'fi' >>$myfile

    # run pre app routine
    echo ". $GEMPIS_EXEDIR/gempis_preapp" >>$myfile

    # Now say that I am calling the command
    echo "#" >> $myfile
    printf 'verbose "AppWrap.`hostname`($$): ' >>$myfile
    printf "Starting [$exec $MyVersion " >>$myfile
    printf '$* ' >>$myfile
    echo "$args].\"" >>$myfile

    # additional options passed by the mpirun command also need to be passed
    # to the calling routine. This is the purpose of the $* option.
    #echo "set -x" >>$myfile
    echo "# Calling command" >> $myfile
    printf "$EXEDIR/gempis_send $GEMPIS_SUBMISSIONHOST $GEMPIS_COMMUNICATIONPORT " >>$myfile
    printf "$GEMPIS_MPI_RUN $exec $MyVersion " >>$myfile
    printf '$* ' >>$myfile
    echo "$args" >> $myfile
    echo 'result=$?' >> $myfile

    # write to a file from each instance 
    # format: "<result> <hostname> <pid>"
    # this is done for two reasons: 
    #  - gives an individual return value, and 
    #  - indicates normal termination (not core dump or termination signal)
    echo "#" >> $myfile
    echo 'verbose "AppWrap.`hostname`($$): Returned [$result]"' >>$myfile
    echo 'echo "$result" >> ${GEMPIS_APPWRAPPATH}_`hostname`_$$.result' >>$myfile

    # run post app routine, and exit with return value
    echo '. $GEMPIS_EXEDIR/gempis_postapp $result' >>$myfile
    echo 'exit $result' >>$myfile

    # make file executable
    verbose "Changing file [`basename $myfile`] mode to executable."
    chmod +x $myfile
}

############################################################################
#
# main
#

# if no parameters then display usage message
[ $# -lt 1 ] && usage

# load system and user environment settings
load_definitions

#
#verbose "Defining [`uname`] PRE trap handler."
verbose "Defining [`uname`] trap handler."
#trap pre_trap_handler INT TERM QUIT HUP
trap trap_handler INT QUIT HUP

# define cfg stuff
TMPDIR=$GEMPIS_PROGNAME
FILENAME=${GEMPIS_PROGNAME}-${GEMPIS_USERNAME}.tar
OUTFILE=`pwd`/$FILENAME

# check if help has been asked
if [ "$1" = "help" ]; then
    show_help
    exit 0
fi

# load resource definitions
load_resources

# define resource name
define_resource $1

# load jobtype definitions
load_jobtype

#############################################################################
#
# If configuration indicates that the command has to go to a Head Node
# and I am not this computer, the pass the command along.
#

if [ -n "$GEMPIS_HEADNODE_LIST" ]; then

    # check that the cwd is not a local disk
    verbose "A remote connection is requested, checking working directory."
    local_disk=`is_cwd_local_disk`
    if [ -n "$local_disk" ]; then
	terminate ERROR "The current working directory is a local disk. 
    Data used on a remote computer must be on a shared disk."
    else
	verbose "CWD is on a shared disk."
    fi

    # find a valid headnode
    verbose "Headnode is defined, checking connections."
    find_headnode

    # am I NOT the headnode?
    if [ "$GEMPIS_HOSTNAME" != "`echo $GEMPIS_HEADNODE | awk -F'.' '{print $1}'`" ]; then

	# forward the command to the head node
	send_gempis_command $*

	# terminate afterward
	exit $result
    fi
fi

#############################################################################
#
# If we are here then the command will be run locally
#

#verbose "Defining [`uname`] trap handler."
#trap trap_handler INT TERM QUIT HUP

# save local operating system name
myos=`uname`
verbose "My operating system is [$myos]."

# check that the data directory is in order
check_datadir

# check that the Job Type is correctly setup
verbose "Verifying that Job Type environment has been correctly set up."
jobtype_verify

#############################################################################
#
# if no more arguments, the display resource availability and exit

verbose "Checking again parameter [$1] for command."
if [ "$1" = "res" ]; then
    shift
    verbose "Starting gempis_resource 1 [$*]"
    . $GEMPIS_EXEDIR/gempis_resource $*
    exit $?
fi
if [ "$1" = "resources" ]; then
    verbose "Starting gempis_resource 2 [$*]"
    . $GEMPIS_EXEDIR/gempis_cfg resource
    exit $?
fi
#
if [ -z "$2" -o "$2" = "loop" -o "$2" = "daemon" -o "$2" = "client" ]; then
    verbose "Starting gempis_resource 3 [$*]"
    . $GEMPIS_EXEDIR/gempis_resource $*
    exit $?
fi

# the resource name has been defined, remove for further processing
shift


##################################################
#
# parse the rest of the arguments
#

# number of CPUs, machine file name, and command to run
export GEMPIS_NUMCPUS=$1
shift
if [ `echo $GEMPIS_NUMCPUS | awk '{ printf("%d", $1) }'` = 0 ]; then
    terminate ERROR "Invalid number of processors: [$GEMPIS_NUMCPUS]."
else
    verbose "This job has requested to run on [$GEMPIS_NUMCPUS] processors."
fi

# remove the run time estimate if it is there
case "$1" in
*:*) runtimefound="true";shift;;
esac

if [ "${runtimefound}" = "" -a "$GEMPIS_RESOURCE" != "Darwin" ]; then
   echo ""
   echo "****"
   echo "**** Run time estimate is missing, please specify one next time."
   echo "**** Using default value (01:00) for now."
   echo "**** Syntax: gempis -$CAULDRON_VERSION CLUSTERNAME NUMCPUS HH:MM ..."
   echo "**** Only required for v2012.10 and later releases."
   echo "****"
   echo ""
fi

# parse command, extracting executable, options and mpi options
export GEMPIS_EXEC="$1"
shift

export GEMPIS_ARGS="$*"
verbose "The command is [$GEMPIS_EXEC $GEMPIS_ARGS]."

# at this point we are ready to start a job
send_message STATE "START"

#############################################################################
#
# verify parameters
#

# set the name of the executable
export GEMPIS_EXECNAME=`basename $GEMPIS_EXEC`

# check that executable exists
verify_exec $GEMPIS_EXEC

# check that the resource name exists
[ -z "$GEMPIS_RESOURCE" ] && terminate ERROR "Resource name not defined for index [$GEMPIS_RES_INDEX]."

#############################################################################
#
# generate a job id
#
send_message STATE "ACCEPT"

# gempis job id
export GEMPIS_ID=`make_jobid`
# job type
export GEMPIS_JOBTYPE="${res_jobtype[$GEMPIS_RES_INDEX]}"
# mpi reference name
export GEMPIS_MPIREF="${res_mpiref[$GEMPIS_RES_INDEX]}"
# job name
export GEMPIS_JOBNAME=${GEMPIS_EXECNAME}-${GEMPIS_RESOURCE}-${GEMPIS_ID}
# directory for temporary job files
export GEMPIS_JOBPATH=${GEMPIS_WORKDIR}/${GEMPIS_JOBNAME}
# log filename
export GEMPIS_LOGFILE=$GEMPIS_JOBPATH/logfile.log
# error filename
export GEMPIS_ERRFILE=$GEMPIS_JOBPATH/logfile.err
# name of machine file. 
export GEMPIS_MACHINEFILE=$GEMPIS_JOBPATH/machinefile
# name of application wrapper file 
export GEMPIS_APPWRAPPATH="${GEMPIS_JOBPATH}/gempis_appwrapper"

# add a line to the job start log file
if [ -n "$GEMPIS_PARENT" ]; then
    echo "$GEMPIS_ID|`get_datetime`|$GEMPIS_USERNAME|$GEMPIS_PARENT|$GEMPIS_RESOURCE|$GEMPIS_NUMCPUS|" >> $GEMPIS_TMPDIR/START.log
else
    echo "$GEMPIS_ID|`get_datetime`|$GEMPIS_USERNAME|$GEMPIS_HOSTNAME|$GEMPIS_RESOURCE|$GEMPIS_NUMCPUS|" >> $GEMPIS_TMPDIR/START.log
fi
chmod 666 $GEMPIS_TMPDIR/START.log 2>/dev/null

# add a line to the users job start log file
echo "$GEMPIS_RESOURCE|$GEMPIS_ID|`get_datetime`|$GEMPIS_CWD|$GEMPIS_EXEC $GEMPIS_ARGS|$GEMPIS_HOSTNAME|$GEMPIS_NUMCPUS|" >> $GEMPIS_USERDIR/user-START.log

# send out my Gempis id
# this is backwards from what it should be 
send_message JOBID $GEMPIS_ID

# create job directory
verbose "Creating work directory [$GEMPIS_JOBPATH]."
mkdir -p $GEMPIS_JOBPATH
[ ! -d $GEMPIS_JOBPATH ] && terminate ERROR "Path [$GEMPIS_JOBPATH] could not be created."
chmod 777 $GEMPIS_JOBPATH

# load and save MPI references
load_mpirefs
define_mpiref
verbose "Defining [`uname`] PRE trap handler."

# LSFHPC jobtype will skip the Gempis mpiwrapper so 
# MPI definitions must be done here
if [ $GEMPIS_JOBTYPE = "LSFHPC" ]; then
    verbose "MPI reference name is [$GEMPIS_MPI_NAME]."

    [ -n "$GEMPIS_MPI_OS" ] && verbose "MPI reference runs on [$GEMPIS_MPI_OS]."

    [ -n "$GEMPIS_MPI_PATH" ] && verbose "MPI path is [$GEMPIS_MPI_PATH]."
# Cannot do this because fastcauldron is then directly found
#    if [ -n "$GEMPIS_MPI_PATH" ]; then
#       # prepend the executable location to path
#	verbose "Prepending MPI executables in [$GEMPIS_MPI_PATH] to PATH."
#	export PATH="$GEMPIS_MPI_PATH:$PATH"
#    fi

    if [ -n "$GEMPIS_MPI_LIB" ]; then
        # add library to LD_LIBRARY_PATH
	verbose "MPI libraries in [$GEMPIS_MPI_LIB] added to LD_LIBRARY_PATH."
	if [ -z "$LD_LIBRARY_PATH" ]; then
	    export LD_LIBRARY_PATH="${GEMPIS_MPI_LIB}"
	else
	    export LD_LIBRARY_PATH="${GEMPIS_MPI_LIB}:${LD_LIBRARY_PATH}"
	fi
    fi
 
    [ -n "$GEMPIS_MPI_START" ] && verbose "MPI start command is [$GEMPIS_MPI_START]."
    [ -n "$GEMPIS_MPI_STOP" ] && verbose "MPI stop command is [$GEMPIS_MPI_STOP]."
    [ -n "$GEMPIS_MPI_RUN" ] && verbose "MPI run command is [$GEMPIS_MPI_RUN]."
fi

export GEMPIS_SUBMISSIONHOST=`hostname -f`
# export GEMPIS_SUBMISSIONHOST="amsd2a-n-b01001.europe.shell.com"
verbose "starting gempis receiver"
$EXEDIR/gempis_recv 7788 /tmp/port$$
verbose "retrieving port id"
export GEMPIS_COMMUNICATIONPORT=`cat /tmp/port$$`
# export GEMPIS_COMMUNICATIONPORT=7788
# /bin/rm -f /tmp/port$$
verbose "retrieved port id = $GEMPIS_COMMUNICATIONPORT"

# build the application wrapper script for the application
build_appwrapper $GEMPIS_APPWRAPPATH $GEMPIS_EXEC $GEMPIS_ARGS $GEMPIS_EXEC_OPTIONS


#############################################################################
#
# run any prejob stuff
#

# run mpiref pre-job routine
verbose "Running MPI ${GEMPIS_MPIREF} prejob subroutine."
mpi_prejob_${GEMPIS_MPIREF}

# if there is a pre job handler, then run it
if [ -n "$GEMPIS_PREJOB" ]; then
    verbose "Running defined Gempis prejob routine [$GEMPIS_PREJOB]."
    $GEMPIS_PREJOB
fi

#############################################################################
#
# submit job to the appropriate job type handler
#
send_message STATE "SUBMIT"

# execute the command to submit the job
jobtype_submit_command
result=$?
verbose "Job submit completed with result = [$result]."

# is the error number if the same as 255 (SSH failure) then change it to 254
if [ $result = "255" ]; then
    verbose "Changing return value to a non SSH error (254)."
    result=254
fi

# translate the return value into something users can understand
case $result in 
0)
	GEMPIS_RESULT="SUCCESS"
	;;
1)
	GEMPIS_RESULT="ERROR"
	;;
2)
	GEMPIS_RESULT="ABORT"
	;;
*)
	GEMPIS_RESULT="ERROR"
	;;
esac
export GEMPIS_RESULT
verbose "The final result is [$GEMPIS_RESULT]."

# execute post job processing
. $GEMPIS_EXEDIR/gempis_postjob

# terminate with result
exit $result

# End script
