#
# gempis_utils - common utilities used by gempis scripts
#
# Dependencies:
#   GEMPIS_OS
#   GEMPIS_VERBOSE
#   GEMPIS_SSH
#   GEMPIS_PID
#   GEMPIS_TMPDIR
#   GEMPIS_DATADIR
#   GEMPIS_JOBTYPE
#
# Called by:
#   gempis_submit
#   gempis_mpiwrapper
#   <appwrapper>
#
# Notes:
# 
############################################################################


############################################################################
#
# shows formatted date/time, also suitable for file names
#
# Syntax: var=`get_datetime`
#
# Returns: nothing
#
# Output: date/time in format:
#   YYYY.mm.dd-HH:MM:SS
#
get_datetime()
{
    echo "`date '+%Y.%m.%d-%T'`"
}

############################################################################
#
# sends a gempis status message
#
# Syntax: send_messsage <message_type> <message>
#         message does not need to be in quotes
#
# Returns: nothing
#
# Output:
#  MeSsAgE <message>
#
# Notes:
# 1. GEMPIS_FORMAT_MESSAGE is set to yes by interactive applications so 
#    they can filter information from standard out.
# 2. GEMPIS_SHOWWHERE is used for debugging messages to find out where a 
#    message came from. 
#
send_message()
{
    type=$1
    shift

    # display format when when running gempis tools
    [ -n "$GEMPIS_FORMAT_MESSAGES" ] && echo -n "MeSsAgE "

    # display message type
    echo -n "$type "

    # when set display where the message came from 
    [ -n "$GEMPIS_SHOWWHERE" ] && echo -n "(`hostname`) "

    # display message
    echo "$*"
}

############################################################################
#
# print a verbose message when activated
#
# Syntax: verbose "<message>"
#         the message must be in quotes
#
# Returns: nothing
#GEMPIS_SEND_NOFORMAT
# Output: when GEMPIS_VERBOSE is defined
#  MeSsAgE VERBOSE     <message>
#
verbose()
{
    [ -n "$GEMPIS_VERBOSE" ] && send_message VERBOSE "   ${1}"
}

############################################################################
#
# shows the help page for the application
#
show_help()
{
    verbose "Dumping help file [$GEMPIS_DOCDIR/$GEMPIS_PROGNAME.txt]."
    cat $GEMPIS_DOCDIR/$GEMPIS_PROGNAME.txt
}

############################################################################
#
# creates an GEMPIS jobid for this job 
#
# Syntax: jobid=`make_jobid`
#
# Returns: nothing
#
# Output: jobid value
#      jobid format, 8 digit 0 preceeded decimal
#
# don't put any debug stuff in here, output redirect to jobid value
#
make_jobid()
{
    # is a database directory defined
    result=1
    if [ -n "$GEMPIS_TMPDIR" ]; then

	jobid=`$GEMPIS_EXEDIR/getnextjobid $GEMPIS_TMPDIR/lastid $GEMPIS_TMPDIR/lastid.lock`
	result=$?
    fi

    # if above procedure was not done or it failed
    if [ $result != 0 -o -z "$jobid" ]; then

	# make up a number
	let 'jobid = RANDOM + PID'
    fi

    # write value
    echo $jobid | awk '{ printf( "%08d", $1 ) }'
}

#############################################################################
#
# determines the number of CPU's on the local machine
#
# Syntax: cpus=`get_my_cpus`
#
# Returns: nothing
#
# Output: the number of processors on the computer
# Format: repeats of the following for each processor
#processor       : 0
#vendor_id       : AuthenticAMD
#cpu family      : 15
#model           : 65
#model name      : Dual-Core AMD Opteron(tm) Processor 2220
#physical id     : 0
#siblings        : 2
#core id         : 0
#
get_my_cpus()
{
    mycpus=`grep ^processor /proc/cpuinfo | wc -l | awk '{ print $1 }'`

    # assume at least one processor
    [ -z "$mycpus" -o $mycpus = 0 ] && mycpus=1
    echo $mycpus
}

#############################################################################
#
# determines the amount of memory in MB on the local machine
#
# Syntax: memory=`get_my_memory`
#
# Returns: nothing
#
# Output: memory in MB
# Format: top of the output (GL4!!! Mem line is different)
#        total:    used:    free:  shared: buffers:  cached:
#Mem:  16461172736 7355109376 9106063360        0 128552960 6322589696
#Swap: 38651936768        0 38651936768
#MemTotal:     16075364 kB
#MemFree:       8892640 kB
#
get_my_memory()
{
    mymemory=`grep 'MemTotal' /proc/meminfo | awk '{ printf( "%d\n", $2 / 1024)}'`

    # display result
    echo $mymemory
}

#############################################################################
#
# checks the data directory
#
# Syntax: check_datadir
#
# Returns: nothing
#
# Output: none
#
# will create the directory if it doesn't exist. So only use if you need it.
#
check_datadir()
{
    if [ -z "$GEMPIS_DATADIR" ] ; then
	return
    fi

    # define data directory, if it doesn't exist, create it
    if [ ! -d $GEMPIS_DATADIR ]; then

	verbose "Creating [$GEMPIS_JOBTYPE] data directory [$GEMPIS_DATADIR]."
	mkdir -p $GEMPIS_DATADIR

	# make accessable to everyone
	chmod -R 777 $GEMPIS_DATADIR
    fi
}

############################################################################
#
# determines whether the cwd is a local or shared disk
#
# Syntax: ans=`is_cwd_local_disk`
#
# Returns: nothing
#
# Output: "YES" - the disk is local, 
#         ""    - the disk is shared
#
is_cwd_local_disk()
{
    local_cwd=`df -l . 2>/dev/null | tail -n +2`
    [ -n "$local_cwd" ] && echo "YES" 
}

############################################################################
#
# interrupt handler subroutine, for application wrapper
#
# Syntax: trap appwrap_trap_handler <signal list>
#
# Returns: terminates execution (result 2)
#
# Output: Displays a GEMPIS warning message before termination
#
appwrap_trap_handler()
{
    # inform that there was an interrupt
    send_message WARNING "User interrupt."

    # terminate program
    exit 2
}

############################################################################
#
# terminate gempis while it was setting up a job. The jobs has not actually 
# been started. Show an error message in a format that will be used in the 
# processors field of the JobLauncher window
#
# Syntax: terminate [ABORT|ERROR] <error message, may be multi-lined>
#    ABORT - user has terminated operation
#    ERROR - an error caused termination
#
# ------------------------------------------------
# not running in gempis
#
# Returns: doesn't return. exits with an error value
#
# Output: text string
#   <error message>
# ------------------------------------------------
# JOBID not defined
#
# Returns: doesn't return. exits with an error value
#
# Output: (format is defined so it will display in the gempil resource window)
#   MeSsAgE DATA TOTAL ?
#   MeSsAgE DATA AVAILABLE ?
#   MeSsAgE DATA MEMORYPER ?
#   MeSsAgE BLOCK INUSEBY
#   <error message>
#   MeSsAgE BLOCKEND 
# ------------------------------------------------
# job id is defined
#
# Returns: doesn't return. exits with an error value: 
#    1 for abort, 
#    2 for error
#
# Output: (format is defined so it will display in the gempil resource window)
#   MeSsAgE DATA TOTAL ?
#   MeSsAgE DATA AVAILABLE ?
#   MeSsAgE DATA MEMORYPER ?
#   MeSsAgE BLOCK INUSEBY
#   <error message>
#   MeSsAgE BLOCKEND 
# ------------------------------------------------
#
terminate()
{
    # set result to error type, needed by postjob
    export GEMPIS_RESULT="$1"

    # has gempis not been called as the application, 
    # then this is a support application, 
    if [ "$GEMPIS_PROGNAME" != "gempis" ]; then

        # don't use the gempis data format
	echo "Failure in application: $1"
	shift
	echo "$*"
	echo ""
	echo "$GEMPIS_PROGNAME exiting..."
	echo ""
	echo ""
	exit 1
    fi

    # is a job id has not been defined 
    if [ -z "$GEMPIS_ID" ]; then

        # we are only setting up the job
	# write format in such a way that it can be shown in gempil

	if [ -n "$GEMPIS_FORMAT_MESSAGES" ]; then
            # blank out total, available and memory per fields
	    send_message DATA TOTAL '?'
	    send_message DATA AVAILABLE '?'
	    send_message DATA MEMORYPER '?'

            # start of block message, first line is blocked
	    send_message BLOCK INUSEBY
	    echo ""
	fi

        # send message
	echo "$2"

	if [ -n "$GEMPIS_FORMAT_MESSAGES" ]; then
            # end of block message
	    send_message BLOCKEND
	fi
    else

        # tell thme we are shutting down
	send_message STATE "TERMINATE"

	# the job was defined 
	if [ -n "$GEMPIS_FORMAT_MESSAGES" ]; then
            # blank out total, available and memory per fields
	    send_message DATA TOTAL '?'
	    send_message DATA AVAILABLE '?'
	    send_message DATA MEMORYPER '?'

            # start of block message, first line is blocked
	    send_message BLOCK INUSEBY
	    echo ""
	fi

        # first display cause of going into terminate state
	send_message ERROR "$2"

	if [ -n "$GEMPIS_FORMAT_MESSAGES" ]; then
            # end of block message
	    send_message BLOCKEND
	fi

        # execute post processing
	. $GEMPIS_EXEDIR/gempis_postjob
    fi

    # terminate execution
    exit 2
}


############################################################################
#
# verifies that an executable can be found
#
# Syntax: verify_exec <executable>
# 
# Returns: will return if successful
#          otherwise, will generate an error message and exit
# 
# Notes: 
# - Solaris returns a 0 on fail of which.
verify_exec()
{
    exe=${1}

    # determine the full path to the exec
    wordcount=`which --skip-alias $exe 2>&1 | wc -w`

    # a failure will return "no $exe in ...." many words
    # a success will return only one word
    [ "$wordcount" -ne 1 ] && terminate ERROR "Executable [${exe}] not found in [$PATH]." 
}

############################################################################
#
# terminates a process id (pid)
#
# Syntax: kill_pid <pid>
# 
# Returns: 0 if successful
#          otherwise, will generate an error message and return 1
# 
# Notes: 
kill_pid()
{
    pid=$1
    verbose "Killing pid [$pid]."
    kill $pid
    res=$?
    if [ ! $res = 0 ]; then
	echo "ERROR! Process [$pid] could not be killed."
	return 1
    fi
    return 0
}

############################################################################
#
# loads system and user definitions
#
# Parameters:
# $1 - if defined will skip user definitions
# 
# Returns: nothing, but will terminate on error
#
load_definitions()
{
    skip=$1

    # check that system definition file has been found, and execute
    if [ ! -f $GEMPIS_SYSTEM_DEF ]; then
	terminate ERROR "System definition file 
    [$GEMPIS_SYSTEM_DEF] 
    not found."

    else
	verbose "Running system definition file [$GEMPIS_SYSTEM_DEF]."
	. $GEMPIS_SYSTEM_DEF
	[ $? -ne 0 ] && terminate ERROR "Error in GEMPIS SYSTEM definition file 
    [$GEMPIS_SYSTEM_DEF]."

    fi

    if [ -n "$skip" ]; then
	verbose "Skipping user definitions"
	return
    fi

    # define user definitions if they exist 
    if [ -f $GEMPIS_USER_DEF ]; then

	verbose "Running user definition file [$GEMPIS_USER_DEF]."

	. $GEMPIS_USER_DEF
	[ $? -ne 0 ] && terminate ERROR "Error in GEMPIS USER definition file 
    [$GEMPIS_USER_DEF]."

    else
	verbose "User definition file [$GEMPIS_USER_DEF] has not been defined."
    fi
}

############################################################################
#
# loads system and user mpi references
#
# Parameters: none
# 
# Returns: nothing, but will terminate on error
#
# These configuration files refer to variables that need to be defined
#
load_mpirefs()
{
    # check and load default MPI references
    if [ ! -f $GEMPIS_MPIREF_FILE ]; then

	echo "Default MPIREF definition file 
    [$GEMPIS_MPIREF_FILE] 
    not found."

	exit 1
    else

	verbose "Running system MPIREF definition file [$GEMPIS_MPIREF_FILE]."

	. $GEMPIS_MPIREF_FILE
	[ $? -ne 0 ] && terminate ERROR "Error in GEMPIS MPIREF definition file 
    [$GEMPIS_MPIREF_FILE]."

    fi

    # if user has a MPIREF definition file, use it
    if [ -f $GEMPIS_MPIREF_USER ]; then

	verbose "Running user MPIREF file [$GEMPIS_MPIREF_USER]."

	. $GEMPIS_MPIREF_USER
	[ $? -ne 0 ] && send_message WARNING "Error in GEMPIS USER MPIREF file 
    [$GEMPIS_MPIREF_USER]."

    else
	verbose "User MPIREF file [$GEMPIS_MPIREF_USER] not found."
    fi
}

############################################################################
# 
# check that mpi reference is valid and sets environment variables
#
# uses GEMPIS_MPIREF to refer to the reference being searched
#
define_mpiref()
{
    # get MPI information from the resource
    i=1
    while [ $i -lt $GEMPIS_MAXMPIREF ]; do
	if [ "${mpi_name[$i]}" = "$GEMPIS_MPIREF" ]; then
	    break
	fi
	let 'i = i + 1'
    done
    [ $i = "$GEMPIS_MAXMPIREF" ] && terminate ERROR "MPI reference 
    [$GEMPIS_MPIREF] 
    does not exist."

    export GEMPIS_MPI_INDEX=$i
    verbose "MPI reference [$GEMPIS_MPIREF] has an index of [$GEMPIS_MPI_INDEX]."

    # save shortened MPI reference names and check that MPI will run on this platform
    mpios="${mpi_os[$GEMPIS_MPI_INDEX]}"
    [ -z "$mpios" ] && terminate ERROR "MPI reference 
    [$GEMPIS_MPIREF] 
    does not define an operating system for resource 
    [$GEMPIS_RESOURCE]."

    # save shortened MPI reference names
    export GEMPIS_MPI_NAME="${mpi_name[$GEMPIS_MPI_INDEX]}"
    export GEMPIS_MPI_OS="${mpi_os[$GEMPIS_MPI_INDEX]}"
    export GEMPIS_MPI_PATH="${mpi_path[$GEMPIS_MPI_INDEX]}"
    export GEMPIS_MPI_LIB="${mpi_lib[$GEMPIS_MPI_INDEX]}"
    export GEMPIS_MPI_START="${mpi_start[$GEMPIS_MPI_INDEX]}"
    export GEMPIS_MPI_STOP="${mpi_stop[$GEMPIS_MPI_INDEX]}"
    export GEMPIS_MPI_RUN="${mpi_run[$GEMPIS_MPI_INDEX]}"

    # need to add path here because our current MPICH is in the CBM
    # executable directory
    [ -n "$GEMPIS_MPI_START" -a -n "$GEMPIS_MPI_PATH" ] && export GEMPIS_MPI_START="$GEMPIS_MPI_PATH/$GEMPIS_MPI_START"
    [ -n "$GEMPIS_MPI_STOP" -a -n "$GEMPIS_MPI_PATH" ] && export GEMPIS_MPI_STOP="$GEMPIS_MPI_PATH/$GEMPIS_MPI_STOP"
    [ -n "$GEMPIS_MPI_RUN" -a -n "$GEMPIS_MPI_PATH" ] && export GEMPIS_MPI_RUN="$GEMPIS_MPI_PATH/$GEMPIS_MPI_RUN"
}

############################################################################
# 
# load system, regional and user resources
#
# Syntax: load_resources [ignore]
#    
# when ignore is defined it will not terminate if user configuration is not 
# defined.
#
# This routine also check to see if the gempis user directory has been 
# created. 
# If not it recomends to the user run the configuration program.
#
load_resources()
{
    ignore="$1"

    #####
    #
    # load the system resources
    #
    if [ -n " $GEMPIS_SYSTEM_CONF" -a ! -f "$GEMPIS_SYSTEM_CONF" ]; then

	terminate ERROR "System resource file 
    [$GEMPIS_SYSTEM_CONF] 
    not found."

    else
	verbose "Running system resource file [$GEMPIS_SYSTEM_CONF]."

	. $GEMPIS_SYSTEM_CONF
	[ $? -ne 0 ] && terminate ERROR "Error in GEMPIS SYSTEM resource file 
    [$GEMPIS_SYSTEM_CONF]."

    fi

    #####
    #
    # load regional resources, if they exist
    #
    # check for my own copy of the regional resources
    myregfile=$GEMPIS_USERDIR/resources_${GEMPIS_REGION_NAME}
    if [ -f "$myregfile" ]; then

	verbose "Running local regional resource file [$myregfile]."

	. $myregfile
	[ $? -ne 0 ] && send_message WARNING "Error in GEMPIS regional resource file 
    [$myregfile]."

    elif [ -n " $GEMPIS_REGION_CONF" -a -f "$GEMPIS_REGION_CONF" ]; then
        # load standard regional resources

	verbose "Running regional resource file [$GEMPIS_REGION_CONF]."

	. $GEMPIS_REGION_CONF
	[ $? -ne 0 ] && send_message WARNING "Error in GEMPIS regional resource file 
    [$GEMPIS_REGION_CONF]."

    else
	# regional definitions do not have to exist
	verbose "Regional definition file [$GEMPIS_REGION_CONF] not found."
    fi

    #####
    #
    # load user resources
    #
    if [ -n "$GEMPIS_USER_CONF" -a -f "$GEMPIS_USER_CONF" ]; then
	verbose "Running user resource file [$GEMPIS_USER_CONF]."

	. $GEMPIS_USER_CONF
	[ $? -ne 0 ] && send_message WARNING "Error in GEMPIS USER resource file 
    [$GEMPIS_USER_CONF]."

    else
	# if not defined then gempis has not been setup for this user
	verbose "User resource file [$GEMPIS_USER_CONF] not found."

	# if told to ignore then do not terminate
	if [ -n "$ignore" ]; then
	    return
	fi

	# If this file has not been found then the user has not setup
	# their environment
	terminate ERROR "
    The GEMPIS user environment has not been setup. 

Run the command

  'gempis adm setup'

or type 'gempis adm help' for details.
"
    fi
}

############################################################################
#
# check if resource name is defined and load values
#
# Parameters: 
#   $1 - resource name
#
# Returns: nothing, on error will terminate
#
define_resource()
{
    # define resource name
    export GEMPIS_RESOURCE=$1

    # determine the index to resource name
    i=1
    while [ $i -lt $GEMPIS_MAXRESOURCE ]; do
	if [ "${res_name[$i]}" = "$GEMPIS_RESOURCE" ]; then
	    break
	fi
	let 'i = i + 1'
    done

    # try again looking for a resource that matches $RESOURCE.`uname`
    if [ $i = "$GEMPIS_MAXRESOURCE" ]; then
	verbose "[$GEMPIS_RESOURCE] not found, looking up resource [$GEMPIS_RESOURCE.`uname`]."
	myres="$GEMPIS_RESOURCE.`uname`"
	i=1
	while [ $i -lt $GEMPIS_MAXRESOURCE ]; do
	    if [ "${res_name[$i]}" = "$myres" ]; then
		break
	    fi
	    let 'i = i + 1'
	done
    fi
    [ $i = "$GEMPIS_MAXRESOURCE" ] && terminate ERROR "Resource 
    $GEMPIS_RESOURCE 
    or 
    $myres 
    has not been found."

    # resource is valid
    export GEMPIS_RES_INDEX=$i
    verbose "Resource [$GEMPIS_RESOURCE] has an index of [$GEMPIS_RES_INDEX]."

    # save resource definitions
    export GEMPIS_JOBTYPE="${res_jobtype[$GEMPIS_RES_INDEX]}"
    export GEMPIS_MPIREF="${res_mpiref[$GEMPIS_RES_INDEX]}"
    export GEMPIS_HEADNODE_LIST="${res_remote[$GEMPIS_RES_INDEX]}"
    export GEMPIS_REQUEST="${res_request[$GEMPIS_RES_INDEX]}"
    if [ "$GEMPIS_RESOURCE" = "Darwin" ]; then
       export GEMPIS_SUBMIT="${res_submit[$GEMPIS_RES_INDEX]}"
    else
       export GEMPIS_SUBMIT="${res_submit[$GEMPIS_RES_INDEX]} -We ${GEMPIS_TIME:-01:00}"
    fi
    export GEMPIS_PREPATH="${res_prepath[$GEMPIS_RES_INDEX]}"
    export GEMPIS_CLUSTER="${res_cluster[$GEMPIS_RES_INDEX]}"

    # Check that required parameters have been defined
    [ -z "$GEMPIS_JOBTYPE" ] && terminate ERROR "The Job Type is not defined for resource 
    [$GEMPIS_RESOURCE]."

    [ -z "$GEMPIS_MPIREF" ] && terminate ERROR "MPI reference is not defined for resource 
    [$GEMPIS_RESOURCE]."
}

#############################################################################
# 
# setup definitions for the jobtype
#
# uses GEMPIS_JOBTYPE as reference
#
load_jobtype()
{
    verbose "Setting up jobtype [$GEMPIS_JOBTYPE] definitions."

    verify_exec jobtype_${GEMPIS_JOBTYPE} 

    # setup the jobtype definitions 
    . jobtype_${GEMPIS_JOBTYPE}
    [ $? != 0 ] && terminate ERROR "Jobtype file jobtype_${GEMPIS_JOBTYPE} failed during setup."
}


############################################################################
#
# sorts user environment variables, 
#
# Syntax: dump_user_environment >> outputfile
#
# Only the following environment variables will be accepted:
# - an assignment must be taking place
# - skip line with LOGNAME, Solaris has a problem here
# - skip line with "_=bla, bla" (Solaris last command)
# - skip empty lines
# - skip lines with double quotes (")
# - skip lines with single quotes (')
#
dump_user_environment()
{
    echo "# ------ Begin of inherited environment ------"

    # define a tmp file that will be executed to get variable value
    tmpfile="$GEMPIS_WORKDIR/vars.$GEMPIS_PID"
    touch $tmpfile
    chmod 777 $tmpfile 

    # get the list of environment variables, with the following conditions:
    env | grep "=" | grep -v "^LOGNAME=" | grep -v "^_=" | grep -v "^$" | grep -v '"' | grep -v \' | while read line; do

	variable=`echo $line | awk -F'=' '{ print $1 }'`

	# define file to extract value
        echo "echo $`echo ${variable}`" >$tmpfile

	# output: export VAR="VALUE"
	echo "export $variable=\"`$tmpfile`\""

    done

    # remove temp file
    rm -f $tmpfile

    # say we are finished
    echo "# ------- End of inherited environment -------"
}

############################################################################
#
# evaluates results files created by the appwrapper
#
# Syntax: evaluate_result_files
#
# Returns: = 0  - success,
#          != 0 - failure 
#
# Output: none
#
evaluate_result_files()
{
    verbose "Evaluating the job results."

    # if the result was good check with LSF if that is correct
    resfiles=`ls ${GEMPIS_APPWRAPPATH}*.result 2>/dev/null | wc -w`
    if [ $resfiles = 0 ]; then

	verbose "No application result files exist, possible core dump or termination."
	result=2
	return $result
    fi

    # check if appresult has a result for every CPU
    # cannot be used with intel mpi ...
#    verbose "[$resfiles] of [$GEMPIS_NUMCPUS] processes reported in."
#    if [ $resfiles -lt "$GEMPIS_NUMCPUS" ]; then
#	send_message ERROR "Only [$resfiles] of [$GEMPIS_NUMCPUS] processes reported in."
#	return 2
#    fi

    # check the contents of the results file for a failure
    appres=`cat ${GEMPIS_APPWRAPPATH}*.result | awk '{ print $1 }' | sort -u | tail -1`
    verbose "The highest application result was [$appres]."
    if [ "$appres" = ABORT ]; then
	verbose "Abort detected"
	result=2
    elif [ -n "$appres" ]; then
	result=$appres
    fi

    verbose "Job evaluation result is [$result]."
    return $result
}

############################################################################
#
# tests to see of the machine is reachable
#
# Syntax: test_machine <machine>; result=$?
#
# Returns: = 0  - success,
#          != 0 - failure 
#
# Output: none
#
test_machine()
{
    mach=$1
    command="/bin/ping -n -q -c 1"
    verify_exec `echo "$command" | awk '{ print $1 }'`

    # ping remote machine
    verbose "    [$command $mach]."
    $command $mach >/dev/null 2>&1
    result=$?
    if [ $result = 0 ]; then
	verbose "Connection test successful."
    else
	verbose "Connection test (ping) error [$result]."
    fi
    return $result
}


############################################################################
#
# determines if error could is releated to SSH
#
# Syntax: is_ssh_error $result
#
# Returns: 0 - not ssh error code
#          1 - is an ssh eror code 
#
# SSH Return codes (as of OpenSSH version 3.9)
#     0   - success
#     255 - ssh error
#     127 - directory or file not found
#     otherwise application error
#
is_ssh_error()
{
    result=$1
    [ $result -ne 127 -a $result -ne 255 ] && return 0

    # SSH error: try to deterine the cause (report SSH version)
    verbose "SSH error [$result] detected."
    echo -n "SSH (`$GEMPIS_SSH -V`) Error [$result]. " 
    case $result in
    127)
	    echo "File or directory not found."
	    ;;
    255)
	    echo "Internal ssh error."
	    ;;
    *)
	    echo "Unknown error."
	    ;;
    esac
    return 1
}

############################################################################
#
# executes a command on a remote computer using ssh and checks the result
#
# Syntax: run_ssh "ssh arguments only"
#         Do not put ssh options (-o) in single quotes as this causes ssh to fail.
#
# Returns: result of ssh command
#
# SSH Return codes (as of OpenSSH version 3.9)
#     0   - success
#     255 - ssh error
#     127 - directory or file not found
#     otherwise application error
#
# Output: ssh session (unless otherwise redirected).
#
run_ssh()
{
    ssh_args="$*"
    verbose "Running ssh command [$GEMPIS_SSH $ssh_args]."
    $GEMPIS_SSH $ssh_args 2>&1
    result=$?
    verbose "$GEMPIS_SSH returned [$result]"

    # successful
    if [ $result = 0 ]; then
	verbose "SSH command completed successfully."
	return $result
    fi

    # did the application complete
    is_ssh_error $result
    [ $? = 0 ] && verbose "Application error [$result] detected."

    return $result
}

############################################################################
#
# tests to see of the the user can reach the remote computer using ssh
#
# Syntax: test_ssh <machine>
#
# Returns: 0 - success, 
#          otherwise, error
#
# Output: ""       - successful
#         <string> - error message, ssh failed
#
test_ssh()
{
    remote=$1
    verbose "Testing SSH to node [$remote]."

    # define log file
    tmplog=$GEMPIS_WORKDIR/ssh-test_${GEMPIS_HOSTNAME}_$$.log

    # run ssh command, the out is redirected so we don't see it
    verbose "    [$GEMPIS_SSH -o BatchMode=yes -o StrictHostKeyChecking=no $remote true >$tmplog]"
    run_ssh "-o BatchMode=yes -o StrictHostKeyChecking=no $remote true" >$tmplog
    result=$?

    # did everything go well? then leave
    if [ "$result" = 0 ]; then
	verbose "SSH test successful."
      	rm -f $tmplog >/dev/null
	return 0
    fi

    # dump the log file
    verbose "SSH test failed."
    [ -n "$GEMPIS_VERBOSE" ] && ( 
echo "- Dump of $tmplog: ---------------------"
cat $tmplog
echo "- End ----------------------------------"
)

    # the last line in the ssh command may say something about what went wrong
    # if last line was blank, get the next-to-last
    last_ssh_line=`tail -1 $tmplog` 
    [ -z "$last_ssh_line" ] && last_ssh_line=`tail -2 $tmplog | head -1`

    # check for no or bad configuration
    bad_config=`echo $last_ssh_line | grep "Permission denied"`
    if [ -n "$bad_config" ]; then
	terminate ERROR "SSH is not or incorrectly configured. Enter the command
'gempis adm keygen' to (re)generate your ssh keys."

	# won't get here
	return 1
    fi

    # check for host key error
    host_key=`echo $last_ssh_line | grep "Host key"`
    if [ -n "$host_key" ]; then

	# if no file then something funny is going on
	if [ ! -f ~/.ssh/known_hosts ]; then
	    terminate ERROR "SSH file $HOME/.ssh/known_hosts not found"
	    return 1
	fi

	# save old file, and remove remote from file
	cp  ~/.ssh/known_hosts  ~/.ssh/known_hosts.save
	grep -v $remote ~/.ssh/known_hosts.save > ~/.ssh/known_hosts
	verbose "Host $remote has been remove from file ~/.ssh/known_hosts, please try again"
	return 1
    fi

    verbose "Unknown SSH error."
    return 1
}

############################################################################
# 
# finds a head node in a list os possible candidates
#
# value returned in GEMPIS_HEADNODE
#
# The headnode must be the active one in the list, that will only occur when the 
# primary has fallen over.
#
find_headnode()
{
    # define a temp file for data
    tmpfile="$GEMPIS_WORKDIR/headnode.$GEMPIS_PID"

    for headnode in $GEMPIS_HEADNODE_LIST; do

	# am I the headnode?
	if [ "$GEMPIS_HOSTNAME" = "`echo $headnode | awk -F'.' '{print $1}'`" ]; then

	    # signal success
	    verbose "I am the headnode."
	    echo $headnode > $tmpfile
	    chmod 666 $tmpfile

	    # leave loop 
	    break
	fi

        # test connection to head node
	verbose "Verify [$headnode] is a working head node."

	test_machine $headnode
	result=$?
	[ "$result" != 0 ] && continue

        # test that ssh is working correctly to the head node
	test_ssh $headnode
	result=$?
	[ $result != 0 ] && continue

    	# if we are here then we were successful
	echo "$headnode" > $tmpfile
	break
    done

    # if file was not created then no active headnode was found
    [ ! -f $tmpfile ] && terminate ERROR "Could not detect a working head node in [$GEMPIS_HEADNODE_LIST]."

    # now define the headnode
    export GEMPIS_HEADNODE=`cat $tmpfile`
    verbose "Head node selected = [$GEMPIS_HEADNODE]"
    rm -f $tmpfile
}


############################################################################
#
# builds a script that can run gempis on a remote computer
#
# Description: Some programs using ssh to spawn jobs have the problem that 
# the users environment variables are not available to executables spawned 
# by ssh. This is true even when the users  ~/.ssh/rc script has been 
# defined and executed. 
#
# Syntax: build_gempis_wrapper <wrapper_name>
#
# wrapper_name - name of wrapper script to be executed
# executable - the executable to be run, assumes that executable exists
# arguments  - program arguments
#
build_gempis_wrapper()
{
    myfile=$1
    shift

    # define gempis options to pass on
    options="$*"
    verbose "The gempis options are [$options]."

    # if we are running under CSCE define the version
    [ -z "$GEMPIS_NOT_CSCE" ] && version="-${IBS_VERSION}"

    # start building wrapper script and test that user can write file
    verbose "Building gempis wrapper script $myfile."

    # define execution bash shell header
    echo "#!/bin/bash -lx" > $myfile
    [ $? -ne 0 ] && terminate ERROR "Cannot create gempis wrapper script 
    $myfile. 
    Permission denied."

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

    # now setup environment for this platform
    echo ". /etc/profile" >>$myfile

    # move into my current directory
    echo "#" >> $myfile
    echo "cd $GEMPIS_REM_CWD" >> $myfile
    echo 'result=$?' >> $myfile
    echo '[ "$result" != 0 ] && echo "Gempis Wrapper `hostname`: Move to directory failed."' >>$myfile
    echo 'echo "Moved into [$GEMPIS_REM_CWD]."' >> $myfile

    echo "# Calling command." >> $myfile
    # GEMPIS_PREPEND_COMMAND allows tracing or debugging
    echo "$GEMPIS_PREPEND_COMMAND gempis $version $options" >> $myfile
    echo 'result=$?' >> $myfile

    # finish off file
    echo 'exit $result' >>$myfile

    # make file executable
    verbose "Changing file [`basename $myfile`] mode to executable."
    chmod +x $myfile
}


############################################################################
# 
# sends a gempis options to a remote computer
# 
# Syntax: 
# send_gempis_command $*
#
# builds a wrapper script and uses ssh to a remote computer
#
send_gempis_command()
{
    verbose "Forwarding Gempis command to [$GEMPIS_HEADNODE]."

    # define the parent hostname
    export GEMPIS_PARENT=`hostname`
    sshname="gempis_wrapper-${GEMPIS_RESOURCE}-`get_datetime`-$RANDOM"
    export GEMPIS_SSHSCRIPT="${GEMPIS_WORKDIR}/${sshname}"

    # check if there needs to be a directory translation for remote computer
    GEMPIS_REM_CWD=$GEMPIS_CWD
    GEMPIS_REM_SSHSCRIPT=$GEMPIS_SSHSCRIPT
    if [ -n "$GEMPIS_FROMPATH" -a -n "$GEMPIS_TOPATH" ]; then

	# check inf the current directory has been changed
	gotone=`echo " $GEMPIS_CWD/" | grep " $GEMPIS_FROMPATH/"`
	if [ -n "$gotone" ]; then

	    verbose "Translate remote paths from [$GEMPIS_FROMPATH] to [$GEMPIS_TOPATH]."

	    # convert old path to new path
	    newpath=`echo $GEMPIS_CWD | awk -v r=$GEMPIS_FROMPATH -v s=$GEMPIS_TOPATH '{
  t = $0;
  gsub( r, s, t );
  print t
}'`
	    # set flag indicating the path has changed
	    GEMPIS_REM_CWD=$newpath
	    verbose "    GEMPIS_REM_CWD converted to [$GEMPIS_REM_CWD]"

	    # redefine new location
	    GEMPIS_REM_SSHSCRIPT="${HOME}/${sshname}"
	    verbose "    GEMPIS_REM_SSHSCRIPT converted to [$GEMPIS_REM_SSHSCRIPT]"
	fi
    fi
    export GEMPIS_REM_CWD
    export GEMPIS_REM_SSHSCRIPT

    # build the gempis wrapper script
    build_gempis_wrapper $GEMPIS_SSHSCRIPT `echo $*`

    # run the wrapper on the headnode
    result=1
    for try in 1 2; do

	run_ssh "-X -q -t -o StrictHostKeyChecking=no $GEMPIS_HEADNODE $GEMPIS_REM_SSHSCRIPT"
	result=$?

	# was the result due to the application?
	is_ssh_error $result
	[ $? = 0 ] && break

	verbose "Attempt [$try] failed. Retrying in 30 seconds."
	sleep 30
    done
    verbose "Completed after [$try] attempt(s)."

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
    verbose "Gempis result was [$GEMPIS_RESULT]."

    # cleanup ssh wrapper script
    [ -z "$GEMPIS_DEBUG" ] && rm -f $GEMPIS_SSHSCRIPT

    # if an ssh failure not detected then quit
    is_ssh_error $result
    [ $? = 0 ] && return

    # ssh failure, try to find the cause
    verbose "SSH command failed,"

    [ $result = 255 ] && terminate ERROR "Internal SSH failure connecting to head node [$GEMPIS_HEADNODE]."

    # check if the directory could be seen on the head node
    # Note that we're checking the user directory, not the
    # work (.gempis_tmp) directory, chances are the problem is there.
    verbose "Testing remote directory."
    run_ssh "-o BatchMode=yes -o StrictHostKeyChecking=no $GEMPIS_HEADNODE ls $GEMPIS_REM_CWD" >/dev/null
    result=$?

    # if successful, the file could not be found
    [ $result = 0 ] && terminate ERROR "The wrapper script [$GEMPIS_REM_SSHSCRIPT] 
was not found on the head node [$GEMPIS_HEADNODE]."

    # otherwise, the directory could not be found on the computer
    terminate ERROR "The directory [$GEMPIS_REM_CWD]
probably could not be seen on head node [$GEMPIS_HEADNODE]."
}

############################################################################
#
# define variables used by all gempis environments
#

# define the name of the host, without domain name
export GEMPIS_HOSTNAME=`hostname | awk -F'.' '{ print $1 }'`
# this message appears each time we connect to a new node
verbose "======== Computer [$GEMPIS_HOSTNAME] ========================"

#
# verify that executables required by all off gempis are defined
#
#verbose "Verifying standard executables are in PATH."
# /bin
verify_exec ln
# /usr/bin
verify_exec tail
# possible ucb
verify_exec date
# often a problem
verify_exec awk
# ... and this one
verify_exec sed

# this ensures that files create for gempis are accessable for everyone
umask 0

# define the operating system, the correct ssh command, and the mail program 
export GEMPIS_OS=`uname`
export GEMPIS_SSH="`which ssh`"	
export GEMPIS_MAIL=mail
# define echo without CR
export ECHONOCR="echo -e"

# define the name of the user
export GEMPIS_USERNAME=`whoami`
verbose "I am [$GEMPIS_USERNAME]."

# defines the PID of this process
export GEMPIS_PID=$$
verbose "My PID is [$GEMPIS_PID]."

# export Current Working Directory (CWD)
export GEMPIS_CWD=`pwd`
verbose "Current working directory is [$GEMPIS_CWD]."

# check that required variable is set
[ -z "$IBS_NAMES" ] && terminate ERROR "IBS_NAMES environment variable not defined."
# This definition is used througout gempis to locate gempis system definitions
export GEMPIS_DEFDIR="$IBS_NAMES/gempis"
export GEMPIS_SYSTEM_DEF="$GEMPIS_DEFDIR/definitions_system"

# define and create gempis temporary work directory 
export GEMPIS_WORKDIR=${HOME}/.gempis_tmp
if [ ! -d $GEMPIS_WORKDIR ]; then

    verbose "Creating Gempis work directory [$GEMPIS_WORKDIR]."
    mkdir -p $GEMPIS_WORKDIR
    [ ! -d $GEMPIS_WORKDIR ] && terminate ERROR "Path [$GEMPIS_WORKDIR] could not be created."
    chmod 777 $GEMPIS_WORKDIR
fi
# create temp directory if it does not exist
if [ ! -d $GEMPIS_TMPDIR ]; then
    verbose "Creating Gempis resource temporary directory [$GEMPIS_TMPDIR]."
    mkdir -p $GEMPIS_TMPDIR
    chmod 777 $GEMPIS_TMPDIR
fi
# create database directory if it does not exist
if [ ! -d $GEMPIS_DATABASE ]; then
    verbose "Creating Gempis resource data directory [$GEMPIS_DATABASE]."
    mkdir -p $GEMPIS_DATABASE
    chmod 777 $GEMPIS_DATABASE
fi
# create data directory
if [ ! -d $GEMPIS_DATADIR ]; then
    verbose "Creating Gempis data directory [$GEMPIS_DATADIR]."
    mkdir -p $GEMPIS_DATADIR
    chmod 777 $GEMPIS_DATADIR
fi

#
# End script
