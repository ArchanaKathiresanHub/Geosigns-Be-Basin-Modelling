#
# gempis_postapp - script that is run at the end of each instance of a 
#                  gempis app
#
# Syntax: gempis_postapp <result>
#
# Dependencies:
#
# Called by:
#   gempis_appwrapper
#
# Notes:
# 
############################################################################

# save result value
app_result=$1

#############################################################################
#
# define gempis utilities
#
#. $GEMPIS_EXEDIR/gempis_utils

verbose "gempis_postapp.$GEMPIS_HOSTNAME($$) [$*]."

############################################################################
#
# main
#

# stop sar monitor, if running
if [ -n "$GEMPIS_MONITORAPPS" ]; then
    pid=`ps -ef | grep sar | grep -v grep | grep "$GEMPIS_ID-" | awk '{ print $2 }'`
    if [ -n "$pid" ]; then
	verbose "Killing sar process(es) [$pid]."
	kill $pid >/dev/null 2>&1
    fi
fi

# if there is a post application handler, then run it
if [ -n "$GEMPIS_POSTAPP" ]; then
    verbose "Running post app handler [$GEMPIS_POSTAPP]."
    $GEMPIS_POSTAPP
fi

# sync file system buffers to disk
#verbose "Syncing file system buffers to disk."
#sync

# End script
