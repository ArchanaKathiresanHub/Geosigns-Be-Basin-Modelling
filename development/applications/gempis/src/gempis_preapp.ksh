#
# gempis_preapp - script that is run at the beginning of each instance of 
# an application
#
# Dependencies:
#
# Called by:
#   gempis_appwrapper
#
# Notes:
# 
############################################################################

#############################################################################
#
#
verbose "gempis_preapp.$GEMPIS_HOSTNAME($$) [$*]."

# indicate run is active (will be done on each 
send_message STATE "ACTIVE"

############################################################################
#
# main
#

# if there is a pre application handler, then run it
if [ -n "$GEMPIS_PREAPP" ]; then
    $GEMPIS_PREAPP
fi

# start sar monitor
if [ -n "$GEMPIS_MONITORAPPS" ]; then

    # if one is already running , skip
    gotone=`ps -ef | grep sar | grep -v grep | grep "${GEMPIS_ID}-" | awk '{ print $2 }'`
    if [ -z "$gotone" ]; then

	# save previous directory
	cwd=`pwd`
	cd $GEMPIS_WORKDIR

	GEMPIS_SARFILE="${GEMPIS_ID}-${GEMPIS_HOSTNAME}.sar"
	verbose "Starting SAR monitor for host $GEMPIS_HOSTNAME."

        # max time is 7 days
        #/usr/lib/sa/sadc -F 15 40320 $SARFILE &
	sar -o $GEMPIS_SARFILE 15 40320 >/dev/null 2>&1 &

	# go back to previous directory
	cd $cwd
    fi
fi

# End script
