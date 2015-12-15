#
# gempis_postjob - script that is run at the end of a gempis job
#
# Syntax: gempis_postjob
#
# Dependencies:
#
# Called by:
#   gempis
#
# Notes:
# 1. This script exists because gempis can be terminated in different locations
############################################################################

#############################################################################
#
# define gempis utilities
#
verbose "gempis_postjob [$*]."

# setup the jobtype definitions 
verbose "Setting up jobtype [$GEMPIS_JOBTYPE] definitions."
verify_exec jobtype_${GEMPIS_JOBTYPE} 
. jobtype_${GEMPIS_JOBTYPE}
[ $? != 0 ] && send_message WARNING "Jobtype file [${GEMPIS_JOBTYPE}] failed during setup."

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


############################################################################
#
# main
#

#############################################################################
#
# do any post job processing
#
# if there is a post job handler, then run it
if [ -n "$GEMPIS_POSTJOB" ]; then
    verbose "Running post job handler [$GEMPIS_POSTJOB]."
    $GEMPIS_POSTJOB
fi

# run mpiref post-job routine
verbose "Running MPI [${GEMPIS_MPIREF}] postjob subroutine."
mpi_postjob_${GEMPIS_MPIREF}

# set state to CLEANUP
send_message STATE "CLEANUP"

# now clean this mess up if debugging has not been enabled
if [ -n "$GEMPIS_DEBUG" ]; then
    verbose "GEMPIS_DEBUG variable set, will not cleanup work directory."
else
    # remove JOBPATH directory and everything in it
    if [ -d "$GEMPIS_JOBPATH" ]; then
	verbose "Removing temporary directory [$GEMPIS_JOBPATH]."
	rm -rf $GEMPIS_JOBPATH
    fi

    # cleanup ssh script, if it exists
    if [ -n "$GEMPIS_SSHSCRIPT" -a -f "$GEMPIS_SSHSCRIPT" ]; then
	verbose "Removing ssh script [$GEMPIS_SSHSCRIPT]."
	rm -f $GEMPIS_SSHSCRIPT
    fi
fi

# run jobtype cleanup handler, if defined. 
if [ -n "$GEMPIS_JOBTYPE" ]; then
    verbose "Running jobtype end routine."
    jobtype_end 2>/dev/null
fi

# send any good mail
if [ -n "$GEMPIS_MAIL_SUCCESS" -a "$GEMPIS_RESULT" != "SUCCESS" ]; then
    verbose "Sending success mail to [$GEMPIS_MAIL_SUCCESS]."
    cat $GEMPIS_LOGFILE | $GEMPIS_MAIL -s "Gempis SUCCESS" $GEMPIS_MAIL_SUCCESS
fi

# send any bad mail
if [ -n "$GEMPIS_MAIL_FAILURE" -a "$GEMPIS_RESULT" != "SUCCESS" ]; then
    verbose "Sending failure mail to [$GEMPIS_MAIL_FAILURE]."
    cat $GEMPIS_LOGFILE | $GEMPIS_MAIL -s "Gempis FAILURE" $GEMPIS_MAIL_FAILURE
fi

# add a job line to the end of the system and the user log file
verbose "`hostname` is writing the $GEMPIS_TMPDIR/END.log."
echo "$GEMPIS_RESOURCE|$GEMPIS_ID|`get_datetime`|$GEMPIS_RESULT|" >> $GEMPIS_USERDIR/user-END.log
echo "$GEMPIS_ID|`get_datetime`|$GEMPIS_RESULT|" >> $GEMPIS_TMPDIR/END.log
chmod 666 $GEMPIS_TMPDIR/END.log 2>/dev/null
 
# return result
send_message RESULT $GEMPIS_RESULT

# sign off
send_message STATE "COMPLETE"
 
# End script
