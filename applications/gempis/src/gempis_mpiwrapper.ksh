#
# mpiwrapper - wrapper script for MPI commands
#
# Purpose: a script to generically submit MPI jobs
#
#
# Syntax: mpiwrapper
#
# Input (Environment Variables):
#   GEMPIS_APPWRAPPATH - path to application wrapper
#
# Notes:
# 1. This script is used by each remote ssh process to define the 
# environment variables.
# 2. Will only work when the current working directory is accessable 
# to all of the machines where it will be run.
#
# Dependencies:
#####################################################################

#####################################################################
#gempis_mpiwrapper.ksh
# define gempis utilities
#
. $GEMPIS_EXEDIR/gempis_utils

verbose "gempis_mpiwrapper [$*]."

#######################################################################
#
# stop any MPI environment that may have been started
#
mpiwrapper_verbose()
{
    verbose "MPI Wrapper: $*"
}

#######################################################################
#
# stop any MPI environment that may have been started
#
mpiwrapper_stop()
{
    # if an mpistop has been defined then execute it
    if [ -n "$GEMPIS_MPI_STOP" ]; then 
	mpiwrapper_verbose "Stopping MPI with [$GEMPIS_MPI_STOP]."

	# execute mpi stop routine
	$GEMPIS_MPI_STOP

	mpiwrapper_verbose "The result of the stop was [$?]."
    fi
}

#######################################################################
#
# terminate mpiwrapper execution
#
# $1 message to display
#
mpiwrapper_terminate()
{
    # first display cause of going into terminate state
    send_message ERROR "MPI Wrapper: $*"

    mpiwrapper_stop

    # exit with error
    exit 1
}

############################################################################
#
# interrupt handler subroutine, when INT has been pressed
#
mpiwrapper_trap_handler()
{
    # first display cause of going into terminate state
    send_message ERROR "MPI Wrapper: Interrupt detected by Gempis."

    mpiwrapper_stop

    # exit wit abort indicator
    exit 2
}

#######################################################################
#
# main
#

# define interrupt handler
verbose "Defining [`uname`] trap handler."
trap mpiwrapper_trap_handler INT TERM QUIT HUP

#
mpiwrapper_verbose "Begin [`basename $0` $*]"
if [ -z "$GEMPIS_APPWRAPPATH" ]; then
    mpiwrapper_terminate "Cannot find path to application wrapper."
fi
mpiwrapper_verbose "Application wrapper is [$GEMPIS_APPWRAPPATH]."

#############################################################################
# 
# set definitions for the jobtype
#
mpiwrapper_verbose "Setting up jobtype [$GEMPIS_JOBTYPE] definitions."
verify_exec jobtype_${GEMPIS_JOBTYPE}

# run jobtype setup script   
. jobtype_${GEMPIS_JOBTYPE}
[ $? != 0 ] && mpiwrapper_terminate "Jobtype file jobtype_${GEMPIS_JOBTYPE} failed during setup."

# if a list of hosts or machines needs to created, 
#   determine the computers that will form the cluster
export GEMPIS_TESTLIST=`jobtype_hostlist`
if [ -z "$GEMPIS_TESTLIST" ]; then
    mpiwrapper_terminate "GEMPIS machine list is empty."
else
    mpiwrapper_verbose "The node list is [$GEMPIS_TESTLIST]."
fi

# determine the number of hosts
export GEMPIS_NUMHOSTS=`echo $GEMPIS_TESTLIST | awk '{print NF / 2 }'`
mpiwrapper_verbose "The number of hosts is [$GEMPIS_NUMHOSTS]."

#######################################################################
#
# setup MPI_ stuff
#
# load mpi (subroutine) definitions
mpiwrapper_verbose "Setting up MPI references [$GEMPIS_MPIREF] ."
load_mpirefs
define_mpiref

mpiwrapper_verbose "MPI reference name is [$GEMPIS_MPI_NAME]."

[ -n "$GEMPIS_MPI_OS" ] && mpiwrapper_verbose "MPI reference runs on [$GEMPIS_MPI_OS]."

[ -n "$GEMPIS_MPI_PATH" ] && mpiwrapper_verbose "MPI path is [$GEMPIS_MPI_PATH]."
# Cannot do this because fastcauldron is then directly found
#if [ -n "$GEMPIS_MPI_PATH" ]; then
#    # prepend the executable location to path
#    export PATH="$GEMPIS_MPI_PATH:$PATH"
#    mpiwrapper_verbose "Prepending MPI executables in [$GEMPIS_MPI_PATH] to PATH."
#    mpiwrapper_verbose "PATH is now [$PATH]."
#fi

if [ -n "$GEMPIS_MPI_LIB" ]; then
    # add library to LD_LIBRARY_PATH
    mpiwrapper_verbose "MPI libraries in [$GEMPIS_MPI_LIB] added to LD_LIBRARY_PATH."
    if [ -z "$LD_LIBRARY_PATH" ]; then
	export LD_LIBRARY_PATH="${GEMPIS_MPI_LIB}"
    else
	export LD_LIBRARY_PATH="${GEMPIS_MPI_LIB}:${LD_LIBRARY_PATH}"
    fi
fi

[ -n "$GEMPIS_MPI_START" ] && mpiwrapper_verbose "MPI start command is [$GEMPIS_MPI_START]."
[ -n "$GEMPIS_MPI_STOP" ] && mpiwrapper_verbose "MPI stop command is [$GEMPIS_MPI_STOP]."
[ -n "$GEMPIS_MPI_RUN" ] && mpiwrapper_verbose "MPI run command is [$GEMPIS_MPI_RUN]."

# when running more than one processor
if [ $GEMPIS_NUMCPUS -gt 1 ]; then

    # build machine list file
    mpiwrapper_verbose "Running MPI $GEMPIS_MPIREF buildfile subroutine."
    mpi_buildfile_$GEMPIS_MPIREF
    cpusallocated=$?

    # set friendly permissions
    if [ -f "$GEMPIS_MACHINEFILE" ]; then
    chmod 666 $GEMPIS_MACHINEFILE
fi
fi

# if an MPI_START command has been defined then execute it
if [ -n "$GEMPIS_MPI_START" ]; then 
    mpiwrapper_verbose "Starting MPI with [$GEMPIS_MPI_START]."

    # verify that executable exists
    verify_exec `echo "$GEMPIS_MPI_START" | awk '{ print $1 }'`

    # execute command
    $GEMPIS_MPI_START

    mpiwrapper_verbose "Startup result was [$?]."
fi

#######################################################################

# verify that executable exists
verify_exec `echo "$GEMPIS_MPI_RUN" | awk '{ print $1 }'`

# define the command to execute
command="$GEMPIS_APPWRAPPATH"

# execute command 
mpiwrapper_verbose "Executing [$command]."
$command
result=$?
mpiwrapper_verbose "The command returned [$result]."

# tell them what happened
if [ "$result" != 0 ]; then
    mpiwrapper_terminate "MPI submission failed, result was [$result]"
fi

# stop mpi environment
mpiwrapper_stop

mpiwrapper_verbose "End [`basename $0`]"

exit $result

# End script
