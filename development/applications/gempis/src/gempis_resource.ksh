#
# GEneric MPIrun Submission RESOURCE display
#
# Purpose: a script to shopw generic MPI rsources
#
# Description: 
#
# Syntax: See subroutine usage
#
#############################################################################

# check that this routine has been called in the correct wat
if [ -z "$GEMPIS_USERNAME" ]; then
    echo "Error! Invalid command format, please use 

    gempis res

instead.
"
    exit 1
fi

# name use to locate help page misc/HTML
# set to gempis so it also uses the special format messages
export GEMPIS_PROGNAME="gempis"
verbose "gempis_resource [$*]."


############################################################################
#
# tell user how I work and exit
#
usage()
{
    # terminate application 
    terminate ERROR "Invalid command line. The options are:

    gempis res help
               <resource>
               <resource> loop [<delay>]
               <resource> daemon
               <resource> test
               <resource> killall
               <resource> kill <jobId>
"
}


#############################################################################
#
# verify resources values exist and are correct; create or correct resource file
#
# Output:
#
update_resource_limits() 
{
    verbose "Verifying [$GEMPIS_RESOURCE] resource limits."

    ## file names
    resdir=$GEMPIS_USERDIR/resources
    resfile=$resdir/$GEMPIS_RESOURCE
    tmmpfile=$resdir/$GEMPIS_RESOURCE.tmp

    # create resource directory if it doesn't exist
    resdir="$HOME/.gempis/resources"
    [ ! -d $resdir ] && (
	verbose "Creating resource directory [$resdir]."
	mkdir $resdir
    )

    # update this user's resource total and memory per processor
    reftot=`cat $datfile 2>/dev/null | grep 'MeSsAgE DATA TOTAL' | awk '{ print $4 }'`
    refmem=`cat $datfile 2>/dev/null | grep 'MeSsAgE DATA MEMORYPER' | awk '{ print $4 }'`
    if [ "$reftot" = "?" -o "$refmem" = "?" ]; then
	verbose "Resource is in error."
	if [ -f $resfile ]; then
	    verbose "Removing resource file [$resfile]."
	    rm -f $resfile
	fi
	return
    fi

    # update this user's resource total and memory per processor
    if [ -z "$reftot" -o -z "$refmem" ]; then
	verbose "Resource values are empty, returning."
	return
    fi
    reftot=`echo "$reftot" | awk '{ printf("%d\n", $1) }'`
    refmem=`echo "$refmem" | awk '{ printf("%d\n", $1) }'`
    # if data has been corrupted, return
    if [ "$reftot" = 0 -o "$refmem" = 0 ]; then
	verbose "Resource values are zero, returning."
	return
    fi

    # create resource file if it doesn't exist
    if [ ! -f $resfile ]; then
	verbose "Creating new resource file [$resfile]."
	echo "$reftot $refmem" > $tmmpfile
    else
        # verify that values are correct, otherwise update
	tsttot=`cat $resfile 2>/dev/null | awk '{ printf("%d\n", $1) }'`
	tstmem=`cat $resfile 2>/dev/null | awk '{ printf("%d\n", $2) }'`

	if [ "$tsttot" != "$reftot" -o "$tstmem" != "$refmem" ]; then
	    verbose "Resource value(s) changed:"
	    verbose "   Total processors changed from [$tsttot] to [$reftot]."
	    verbose "   Memory per processor changed  [$tstmem] to [$refmem]."
	    echo "$reftot $refmem" > $tmmpfile
	fi
    fi
    [ -f $tmmpfile ] && (
	mv $tmmpfile $resfile
    )
}

#############################################################################
#
# create resource availability
#
# Output:
#   MeSsAgE DATA TOTAL <total nodes>
#   MeSsAgE DATA AVAILABLE <nodes available>
#   MeSsAgE DATA MEMORYPER <MB memory per processor>
#   MeSsAgE BLOCK INUSEBY
#    JOBID STATE PROCS     USER COMMAND                  START      "grep  
#    <job list>
#   MeSsAgE BLOCKEND
#
create_resource_usage()
{
    # tot=`jobtype_resource_total`
    # [ -z "$tot" ] && tot="3000"
    tot=0
    # mem=`jobtype_resource_memory`
    # [ -z "$mem" ] && mem="16000"
    mem=0
    # avl=`jobtype_resource_available`
    # [ -z "$avl" ] && avl="2800"
    avl=0

    # now display resource values
    if [ $GEMPIS_JOBTYPE = "STANDALONE" ]; then
	verbose "Displaying [$GEMPIS_RESOURCE:`hostname`] resource availability."
    else
	verbose "Displaying [$GEMPIS_RESOURCE] resource availability."
    fi

    # send_message DATA TOTAL "3210"
    # send_message DATA AVAILABLE "2345"
    # send_message DATA MEMORYPER "12121"

    # display in use window or, if values incorrect, display error message
    # only total and memoryper are checked because they're needed by limits.
    send_message BLOCK INUSEBY
    if [ $tot = "?" -o $mem = "?" ]; then

	# add message that there is a problem
	echo "Could not obtain limits for resource [$GEMPIS_RESOURCE]:"
	[ $tot = "?" ] && echo " unknown total, "
	[ $mem = "?" ] && echo " memory failure."
    else
	jobtype_resource_usage
fi
    send_message BLOCKEND
}

#############################################################################
#
# resource server owner functions
#
# the file $ownfile contains the process id of the resource server process

# remove the process owner
remove_owner()
{
    echo "" > $ownfile
    chmod 666 $ownfile
}

# assign my pid as owner of the resource server
assign_owner()
{
    echo $$ > $ownfile
    chmod 666 $ownfile
}

# get pid of the daemion
get_owner()
{
    cat $ownfile 2>/dev/null
}



#############################################################################
#
# determines whether a resource server is currently running. If not then it 
# spawns a new resource server.
#
check_resource_server()
{
    verbose "Checking that resource server is functioning."
    # check that there is a server process defined
    pid=`get_owner`
    if [ -n "$pid" ]; then

        # check that server process is still running
	verbose "Checking that server pid [$pid] is still running."
	tpids=`ps -ef | tail -n +2 | grep -v grep | grep " $pid " | awk '{ print $2 }'`
	found=0
	for tpid in $tpids; do

            # the server process is running
	    if [ "$tpid" = "$pid" ]; then
		verbose "Server $GEMPIS_RESOURCE process still running."
		found=1
	    fi
	done

	if [ $found = 0 ]; then
            # if the server process has not been found, remove owner

	    verbose "Server $GEMPIS_RESOURCE process [$pid] not running, removing owner of [$ownfile]."
	    remove_owner
	fi

    else 
	# no server process is running
	verbose "No resource server detected, taking control."

	# wait a bit 
	sleep 1 

	# check again that a server process has not been defined in the mean time
	pid=`get_owner`
	if [ -z "$pid" ]; then

            # spawn the resource server
	    verbose "Spawning resource [$GEMPIS_RESOURCE] server."
	    nohup $GEMPIS_EXEDIR/gempis_submit res $GEMPIS_RESOURCE daemon >/dev/null 2>&1 &
	else
	    verbose "Another process id [$pid] has take control."
	fi

        # wait before continuing
	sleep 2
    fi
}

#############################################################################
#
# loops in the server function
#
# This loop will only run 100 time then exit. 
#
run_resource_server()
{
    # set flag so output can be read by others
    export GEMPIS_FORMAT_MESSAGES=yes

    # check that another process has not taken control
    pid=`get_owner`
    if [ -n "$pid" ]; then
	tpids=`ps -ef | tail -n +2 | grep -v grep | grep " $pid " | awk '{ print $2 }'`
	found=0
	for tpid in $tpids; do

            # the server process is running
	    if [ "$tpid" = "$pid" ]; then
		verbose "Server $GEMPIS_RESOURCE process [$pid] still running."
		found=1
	    fi
	done

	if [ $found = 1 ]; then
	    verbose "Resource server initiated before I could be started."
	    return
	else
	    remove_owner
	fi
    fi

    # assign myself as the owner
    assign_owner

    # initialize loop count
    loop=0

    verbose "Starting resource server running every $GEMPIS_SLEEP seconds."
    while true; do

	# check that I am still the owner of the lock file
	pid=`get_owner`
	if [ "$pid" != $$ ]; then
	    verbose "Lock file has been taken over by another process, terminating."
	    break
	fi

	# update resource information
	create_resource_usage | tee $tmpfile
	mv $tmpfile $datfile
	chmod 666 $datfile

	# if loop has timed reached limit then exit
	let 'loop = loop + 1'
	if [ $loop -gt 100 ]; then 
	    verbose "Maximum number of loops reached, exiting loop."

            # remove myself as owner
	    remove_owner
	    break
	fi

	# perform wait
	verbose "Sleeping $GEMPIS_SLEEP seconds."
	sleep $GEMPIS_SLEEP
	# a sleep error is an interrupt
	if [ $? != 0 ]; then
	    verbose "Sleep interrupted, exiting loop."
	    break
	fi

    done
}


#############################################################################
#
# displays the data file generated by the server
#
# The client sleeps can be shorter because it does not load a resource
# manager like LSF every time it requests for information.
#
run_resource_client()
{
    verbose "Starting resource client looping every 10 seconds."
    while true; do

	# verify that i caqn still see my parents script still here, 
	#   otherwise the calling side has terminated and deleted the file.
	if [ ! -f $GEMPIS_REM_SSHSCRIPT ]; then
	    verbose "The calling script [$GEMPIS_REM_SSHSCRIPT] has been removed. Terminating..."
	    break
	fi

	# verify that the server for this resource is running
	check_resource_server

	if [ -f $datfile ]; then

	    # display the file
	    cat $datfile

	    update_resource_limits
	fi

        # the client can refresh more often than the server
	verbose "Sleeping 10 seconds."
	sleep 10

	# a sleep error is an interrupt
	if [ $? != 0 ]; then
	    verbose "Sleep interrupted, exiting loop."
	    break
	fi

    done
}


#############################################################################
#
# get resource info for STANDALONE jobtype only
#
run_resource_STANDALONE()
{
    verbose "Starting STANDALONE resource user looping every 10 seconds."

    # set flag so output can be read by others
    export GEMPIS_FORMAT_MESSAGES=yes

    while true; do

        # display resource usage
	create_resource_usage

	verbose "Sleeping 10 seconds."
	sleep 10

	# a sleep error is an interrupt
	if [ $? != 0 ]; then
	    verbose "Sleep interrupted, exiting loop."
	    break
	fi

    done
}


############################################################################
#
# main
#

# if no parameters then display usage message
[ $# -lt 1 ] && usage

# define the system and user environment settings
load_definitions

if [ "$1" = "help" ]; then
    show_help
    exit 0
fi

############################################################################
#
# If we are here then the resource name has been defined
#

# load system and user environment settings
load_definitions

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

# cleanup any old headnode file
if [ -n "$GEMPIS_HEADNODE_LIST" ]; then

    # check that the cwd is not a local disk
    local_disk=`is_cwd_local_disk`
    verbose "Flag for running on local disk is [$local_disk]."
    [ -n "$local_disk" ] && terminate ERROR "The current working directory is on a local disk. 
    Data to be used on a remote computer must exist on a shared disk."

    # find a valid headnode
    find_headnode

    # am I not the headnode?
    if [ "$GEMPIS_HOSTNAME" != "`echo $GEMPIS_HEADNODE | awk -F'.' '{print $1}'`" ]; then
	send_gempis_command res $*
	exit $?
    fi
fi

#############################################################################
#
# From here we are RUNNING ON THE HEADNODE
#

# check that the data directory is in order
check_datadir

# check that the Job Type is correctly setup
verbose "Verifying that Job Type environment has been correctly set up."
jobtype_verify

# determine if the loop sleep time was defined
if [ -n "$3" ]; then
    GEMPIS_SLEEP=$3
else
    GEMPIS_SLEEP=30
fi
export GEMPIS_SLEEP

# define temporary and data files 
tmpfile=$GEMPIS_DATABASE/$GEMPIS_RESOURCE.tmp
datfile=$GEMPIS_DATABASE/$GEMPIS_RESOURCE.dat
ownfile=$GEMPIS_DATABASE/$GEMPIS_RESOURCE.lck

#
# remove my loose resource daemons running for this resource from previous Gempis versions
#
pid=`get_owner`
iam=`whoami`
mypid=$$
#pstree -p -a -c $iam
tpids=`ps -ef | grep gempis_submit | grep daemon | grep $iam | grep -v " $pid " | grep -v " $mypid " | awk '{ print $2 }'`
if [ -n "$tpids" ]; then
    verbose "Daemon owner is [$pid] and I am [$mypid]."
    verbose "The process ids to be terminated are [$tpids]."
#    ps -ef | grep gempis_submit | grep daemon | grep $iam | grep -v " $pid " | grep -v " $mypid "
    for tpid in $tpids; do
	verbose "Terminating a bogus daemon process [$tpid]."
	kill $tpid
    done
fi
# don't remove the data file, that will be cleaned up by someone else

# parse the rest of the arguments
shift
if [ -z "$1" ]; then
    # if we are here then they only want to know about resources usage
    create_resource_usage

    # the STANDALONE resource does not have a specific configuration
    [ $GEMPIS_JOBTYPE != "STANDALONE" ] && update_resource_limits
    exit 0
fi

# check for TEST command
if [ "$1" = "test" ]; then
    list=`jobtype_resource_computers`
    verbose "Resource computer list is [$list]."

    for host in $list; do
	echo "Testing $host..."
	test_machine $host 
	if [ $? -ne 0 ]; then
	    echo "Host failed to respond to a ping."
	else
	   test_ssh $host
           [ $? -ne 0 ] && echo "Host failed to respond to an ssh request."
	fi
    done
    exit $result
fi

# check for KILLALL commands
if [ "$1" = "killall" ]; then

    echo "Terminating all [$GEMPIS_RESOURCE] jobs submitted by user [`whoami`]."
    jobtype_killjob 
    result=$?
    [ $result != 0 ] && echo "ERROR! Could not terminate all jobs."

    echo ""
    echo "Job termination completed. If a separate window is open it may be closed now."

    exit $result
fi

# check for KILL command
if [ "$1" = "kill" ]; then

    jobid=$2
    if [ -z "$jobid" ]; then
	usage
    fi

    echo "Terminating [$GEMPIS_RESOURCE] job [$jobid]."
    jobtype_killjob $jobid
    result=$?
    [ $result != 0 ] && echo "ERROR! Could not terminate [$GEMPIS_RESOURCE] job [$jobid]."

    echo ""
    echo "Job termination completed. If a separate window is open it may be closed now."

    exit $result
fi

# daemon command 
if [ "$1" = "daemon" ]; then

    run_resource_server
    exit 0
fi

# client loop
if [ "$1" = "loop" ]; then

    if [ $GEMPIS_JOBTYPE = "STANDALONE" ]; then
	run_resource_STANDALONE
    else
	run_resource_client
    fi
    exit 0
fi

echo "Error: unknown command [$1]."

# terminate application
exit 1

# End script
