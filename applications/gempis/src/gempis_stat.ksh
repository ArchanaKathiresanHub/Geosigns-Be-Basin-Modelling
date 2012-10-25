#
# This scripts creates a summary of usage for a particular resource 
# for the previous month. An optional parameter can spcify which month 
# should be reported
# 
# Syntax: defined in a users crontab and the user has defined in their 
#         ~/.gempis/definitions_user file:
#
# the purpose of this is to show the effectiveness of the cluster (or not)
#
# location to save gempis_stat data 
#export GEMPIS_STAT_HOME="<home_directory_for_statistics_reporting"
#
# daily collection of <resource> data
#0 20 * * * . /etc/profile; export PATH=/apps/sss/share:$PATH; gempis stat <resource> collect
# report on monthly usage of <resource>
#0 0 6 * * . /etc/profile; export PATH=/apps/sss/share:$PATH; gempis stat <resource> report
#

# check that this routine has been called in the correct wat
if [ -z "$GEMPIS_USERNAME" ]; then
    echo "Error! Invalid command format, please use 

    gempis stat

instead.
"
    exit 1
fi

# name use by CSCE to locate this program
export GEMPIS_PROGNAME="gempis_stat"
verbose "$GEMPIS_PROGNAME [$*]."

# turns off message formatting
export GEMPIS_SEND_NOFORMAT="yes"

# file definitions for resource data collection
export COLLECT_START=./start.tmp
export COLLECT_END=./end.tmp
export FORMATTED_START=./start.log
export FORMATTED_END=./end.log

# file definitions for user data collection
export USER_COLLECT_START=./user-start.tmp
export USER_COLLECT_END=./user-end.tmp
export USER_FORMATTED_START=./user-start.log
export USER_FORMATTED_END=./user-end.log



############################################################################
#
# tell user how I work and exit
#
usage()
{
    # terminate application 
    terminate ERROR "Invalid command line. The options are

    gempis stat help
                <resource> collect
                <resource> resource [<yyyy>-<mm>]
                user
"
}


############################################################################
#
# collect GEMPIS run information from /tmp/gempis
#
# Redhat Linux will remove data stored in /tmp unless it is updated every 720 hours
# so this routine touches everything to make sure they are not removed. 
#
collect_tmp_data()
{
    # update files to make sure they are not deleted
    verbose "Touching file in [$GEMPIS_TMPDIR]."
    touch $GEMPIS_TMPDIR/key
    touch $GEMPIS_TMPDIR/lastid
    touch $GEMPIS_TMPDIR/START.log 
    touch $GEMPIS_TMPDIR/END.log 

    # move into the storage directory
    cd $GEMPIS_STAT_HOME/$GEMPIS_RESOURCE

    verbose "Moving START.log over to [$COLLECT_START]."
    # format out pipe characters
    # set time format so it can be use by the date command
    # format:
    #00030357|2007.12.27-17:07:20|nlgla8|rijkes-n-d99545|Darwin|8|
    cat $GEMPIS_TMPDIR/START.log | grep "|$GEMPIS_RESOURCE|" | awk -F'|' '
{ 
  # split date into day and times
  year = substr( $2, 1, 4 )
  mon = substr( $2, 6, 2 )
  day = substr( $2, 9, 2 )
  tim = substr( $2, 12, 8 )
  printf( "%s %s-%s-%s %s %s %s %s %s\n", $1, year, mon, day, tim, $3, $4, $5, $6 ) 
}' >> $COLLECT_START

    verbose "Sorting start for unique entries in time "
    sort -k2,3 -u $COLLECT_START > xxx.tmp
    mv xxx.tmp $COLLECT_START
    # format
    #00030357 2007-12-27 17:07:20 nlgla8 rijkes-n-d99545 Darwin 4
    chmod 666 $COLLECT_START

    verbose "Moving END.log over to [$COLLECT_END]."
    # sort in jobid order
    # remove pipe characters
    # make sure format is fixed
    # format
    #00019242|2007.08.31-13:16:55|SUCCESS|
    cat $GEMPIS_TMPDIR/END.log | awk -F'|' '
{ 
  # split date into day and time
  year = substr( $2, 1, 4 )
  mon = substr( $2, 6, 2 )
  day = substr( $2, 9, 2 )
  tim = substr( $2, 12, 8 )
  printf( "%s %s-%s-%s %s %s\n", $1, year, mon, day, tim, $3 ) 
}' >> $COLLECT_END

    verbose "Sorting end for unique entries in time "
    sort -k2,3 -u $COLLECT_END > xxx.tmp
    mv xxx.tmp $COLLECT_END
    # format
    #00019241 2007-08-31 12:02:17 SUCCESS
    chmod 666 $COLLECT_END

    # return to where I was
    cd - >/dev/null
    verbose "Collection done."
}


############################################################################
#
# routines that report on resource statistics
#

#
# format files $COLLECT_START and $COLLECT_END, converting time into a timestamp.
#
filter_tmp_files()
{
    # convert start data
    cat $COLLECT_START | while read jobid sdate stime userid host res numcpu; do

	# skip data that does not have all fields
	if [ -n "$numcpu" ]; then
	    # convert time to time stamp value
	    ts=`date --date="$sdate $stime" +%s`

	    echo "$ts $res $host $userid $jobid $sdate $stime $numcpu"
	fi
    done > xxx.tmp
    cat xxx.tmp | sort -r  > $FORMATTED_START
    rm -f xxx.tmp

    # convert end data
    cat $COLLECT_END | while read jobid edate etime result; do

	# skip data that does not have all fields
	if [ -n "$result" ]; then

	    # this section needs to allow for the two different formats for result
	    # 0 or SUCCESS
	    [ $result = SUCCESS ] && result=0 
	    # other number number or ERROR
	    [ $result = ERROR ] && result=1
	    # 2, nothing or ABORT
	    [ $result = ABORT ] && result=2

	    # convert time to time stamp value
	    ts=`date --date="$edate $etime" +%s`

            # format created is 
            #<timestamp> <resource> <host> <user> <jobid> <edate> <etime> <result>
	    echo "$ts unknown unknown unknown $jobid $edate $etime $result" | awk '{ 
printf("%ld %s %s %s %08d %s %s %d\n", $1, $2, $3, $4, $5, $6, $7, $8 ) }'
	fi

    done > $FORMATTED_END
}

#
# generate a line about user usage for the month
#
report_user()
{
    # start line with userid
    user=$1

    # $DURATION_LOG format
    # 00009209 2007-06-06 nlohu0 0 21350
    line=`cat $DURATION_LOG | grep " $user " | grep " ${monthstr}-" | awk 'BEGIN {
success = 0;
fail = 0;
abort = 0;
time = 0;
}
{
  time += $5;
  if ( $4 == 0 )
    success += 1;
  else if ( $4 == 2 )
    abort += 1;
  else
    fail += 1;
}
END {
  printf( "%8d %8d %8d %8d %8d\n", ( time + 1800 ) / 3600, success, fail, abort, 
success+abort+fail );
}'`
    if [ "$line" != "       0        0        0        0        0" ]; then
	printf "%8s %s\n" $user "$line"
    fi
}

#
# generate a monthly report
#
make_resource_report()
{
    if [ -z "$1" ]; then
	monthstr=`date '+%Y-%m' --date="1 month ago"`
    else
	monthstr=$1
    fi

    REPORT=$GEMPIS_STAT_HOME/${GEMPIS_RESOURCE}-$monthstr.txt

    # format file in a way suitable for gempis_parselog
    filter_tmp_files

    # this executable does a faster job of parsing data logs
    $GEMPIS_EXEDIR/parselog > $DURATION_LOG

    # get list of users
    user_list=`cat $DURATION_LOG | awk '{ print $3 }' | sort -u`

    (
        # print title
	echo "Gempis monthly statistics for resource [$GEMPIS_RESOURCE]" 
	echo "" 
	echo "    date `date`"
	echo ""
	echo "Reporting period [$monthstr]"
	echo ""

        # print header
	printf "%8s %8s %8s %8s %8s %8s\n" UserId   Hours Success Failure Aborted Total 

        # for each user
	for user in $user_list; do

	    report_user $user

	done | tee summary.log

	# print a summary of the above
	echo "----------------------------------------------------------------------------"
	cat summary.log | awk '
BEGIN { 
    hours = 0
}
{
    hours += $2 
    suc += $3
    fail += $4 
    abort += $5
    tot += $6
}
END {
    printf( "%-8s %8s %8s %8s %8s %8s\n", "Total", hours, suc, fail, abort, tot );
}'

	cat <<EOF

Data from this period has been stored in 

  $REPORT 

for further investigation.

EOF
    ) | tee $REPORT

    # if a name exists for the support person, then 
    if [ -n "$GEMPIS_EMAIL_HELPDESK" ]; then

        # send this output as mail to users
	echo "e-mailing $GEMPIS_RESOURCE report to [$GEMPIS_EMAIL_HELPDESK]."
	cat $REPORT | $GEMPIS_MAIL -s "gempis stat of resource $GEMPIS_RESOURCE" $GEMPIS_EMAIL_HELPDESK
    fi
}


############################################################################
#
# routines that are used to make the user report
#

#
# generate a line about user use for the month
#
report_resource()
{
    # start line with userid
    resource=$1

    verbose "Reporting on resource [$resource]."
    # $DURATON_LOG format
    # 00009209 2007-06-06 nlohu0 0 21350
    line=`cat $resource.log | egrep " $resource | unknown " | grep " ${monthstr}-" | awk 'BEGIN {
success = 0;
fail = 0;
abort = 0;
time = 0;
}
{
  time += $5;
  if ( $4 == 0 )
    success += 1;
  else if ( $4 == 2 )
    abort += 1;
  else
    fail += 1;
}
END {
  printf( "%8d %8d %8d %8d %8d\n", ( time + 1800 ) / 3600, success, fail, abort, 
success+abort+fail );
}'`
    # if data for resource exists print line
    if [ "$line" != "       0        0        0        0        0" ]; then
	printf "%-16s %s\n" $resource "$line"
    fi
}


#
# collect GEMPIS run information from $HOME/.gempis
#
collect_user_data()
{
    # format out pipe characters
    # set time format so it can be use by the date command
    verbose "Moving user-START.log over to [$USER_COLLECT_START]."
    cat $HOME/.gempis/user-START.log | awk -F'|' -v id=`whoami` '
{ 
  # get the number of processors, set to 4 if not defined
  if ( $7 == "" )
    procs = 4;
  else
    procs = $7

  # get hostname if defined, use unknown if not defined
  if ( $6 == "" )
    host = "unknown"
  else
    host = $6

  # split date into day and time
  year = substr( $3, 1, 4 )
  mon = substr( $3, 6, 2 )
  day = substr( $3, 9, 2 )
  tim = substr( $3, 12, 8 )
  printf( "%s %s %s-%s-%s %s %s %s %s\n", $1, $2, year, mon, day, tim, id, host, procs ) 
}' > $USER_COLLECT_START

    # sort in jobid order
    # remove pipe characters
    # make sure format is fixed
    verbose "Moving user-END.log over to $USER_COLLECT_END."
    cat $HOME/.gempis/user-END.log | sed 's/|/ /g' | awk '
{ 
  if ( NF == 4 ) 
  {
    resource = $1
    id = $2
    result = $4

    # split date into day and time
    year = substr( $3, 1, 4 )
    mon = substr( $3, 6, 2 )
    day = substr( $3, 9, 2 )
    tim = substr( $3, 12, 8 )
  }
  else
  {
    resource = "unknown"
    id = $1
    result = $3

    # split date into day and time
    year = substr( $2, 1, 4 )
    mon = substr( $2, 6, 2 )
    day = substr( $2, 9, 2 )
    tim = substr( $2, 12, 8 )
  }
  printf( "%s %s %s-%s-%s %s %s\n", resource, id, year, mon, day, tim, result ) 

}'> $USER_COLLECT_END

    verbose "User collection done."

    #
    # format files $USER_COLLECT_START and $USER_COLLECT_END, converting time into a timestamp.
    # 

    # convert start data, reverse data
    verbose "Formatting [$USER_COLLECT_START] to [$USER_FORMATTED_START]."
    cat $USER_COLLECT_START | while read res jobid sdate stime userid host numcpu; do

	# skip data that does not have all fields
	if [ -n "$numcpu" ]; then

	    # convert time to time stamp value
	    ts=`date --date="$sdate $stime" +%s`

            # format created is 
            #<timestamp> <resource> <host> <userid> <jobid> <sdate> <stime> <numcpus>
	    echo "$ts $res $host $userid $jobid $sdate $stime $numcpu"
	fi
    done > xxx.tmp
    cat xxx.tmp | sort -r > $USER_FORMATTED_START
    rm -f xxx.tmp

    # convert end data
    verbose "Formatting [$USER_COLLECT_END] to [$USER_FORMATTED_END]."
    cat $USER_COLLECT_END | while read res jobid edate etime result; do

	# skip data that does not have all fields
	if [ -n "$result" ]; then

	    # this section needs to allow for the two different formats for result
	    # 0 or SUCCESS
	    [ $result = SUCCESS ] && result=0 
	    # other number number or ERROR
	    [ $result = ERROR ] && result=1
	    # 2, nothing or ABORT
	    [ $result = ABORT ] && result=2

	    # convert time to time stamp value
	    ts=`date --date="$edate $etime" +%s`

	    [ -z "$result" ] && result=0

            # format created is 
            #<timestamp> <resource> <host> <user> <jobid> <edate> <etime> <result>
	    echo "$ts $res unknown unknown $jobid $edate $etime $result" | awk '{ 
printf("%ld %s %s %s %08d %s %s %d\n", $1, $2, $3, $4, $5, $6, $7, $8 ) }'
	fi

    done > $USER_FORMATTED_END

    verbose "Formatting done."
}

#
# generate user usage statistics
#
# The output is organized by month for the entire period data exists, 
# for all resources detected, alphabetically
#
make_user_report()
{
    if [ -z "$1" ]; then
	monthstr=`date '+%Y.%m' --date="1 month ago"`
    else
	monthstr=$1
    fi

    user=`whoami`
    REPORT=./${user}-${monthstr}.txt

    verbose "Making monthly user report [$REPORT]."

    # collect data
    collect_user_data

    # get list of resources
    resource_list=`cat $USER_FORMATTED_START | awk '{ print $2 }' | sort -u`

    verbose "The resource list is [$resource_list]."
    (
        # print title
	echo "Gempis monthly statistics for user [$user]" 
	echo "" 
	echo "    date `date`"
	echo ""
	echo "Reporting period [$monthstr]"
	echo ""

        # print header
	printf "%-16s %8s %8s %8s %8s %8s\n" Resource Hours Success Failure Aborted Total 

        # for each user
	for resource in $resource_list; do

	    # filter resource out of user files into start.log and end.log for parselog
	    cat $USER_FORMATTED_START | grep " $resource " > start.log
	    cat $USER_FORMATTED_END | grep " $resource " | grep " $monthstr" > end.log

            # this executable does a faster job of parsing data logs
	    $GEMPIS_EXEDIR/parselog > $resource.log

	    report_resource $resource

	done | tee summary.log

	# print a summary of the above
	echo "-----------------------------------------------------------------------------"
	cat summary.log | awk '
BEGIN { 
    hours = 0
}
{
    hours += $2 
    suc += $3
    fail += $4 
    abort += $5
    tot += $6
}
END {
    printf( "%-16s %8s %8s %8s %8s %8s\n", "Total", hours, suc, fail, abort, tot );
}'

        # finish off report
	cat <<EOF

Data from this period has been stored in 

  $REPORT 

for further investigation.

EOF
    ) | tee $REPORT
}



############################################################################
#
# main
#

# define the system and user environment settings
load_definitions

# process some of the simple commands
if [ "$1" = "help" ]; then
    show_help
    exit 0
fi
# create a user report
if [ "$1" = "user" ]; then

    # move into the directory and make report
    if [ -z "$GEMPIS_DEBUG" ]; then
	verbose "Moving into work directory [$GEMPIS_WORKDIR]."
	cd $GEMPIS_WORKDIR
    fi

    # can be done locally
    make_user_report $2

    exit 0
fi

# if no parameters then display usage message
[ $# -lt 2 ] && usage

# load resources
load_resources ignore


# only need to load resources when they are being collected
if [ $2 = collect ]; then
    # define resource
    define_resource $1

    # setup definitions for the jobtype
    load_jobtype
else
    export GEMPIS_RESOURCE=$1
fi

# create data directory if it does not exist
if [ ! -d $GEMPIS_STAT_HOME/$GEMPIS_RESOURCE ]; then
    verbose "Creating collection directory [$GEMPIS_STAT_HOME/$GEMPIS_RESOURCE]."
    mkdir -p $GEMPIS_STAT_HOME/$GEMPIS_RESOURCE
    chmod 777 $GEMPIS_STAT_HOME/$GEMPIS_RESOURCE
fi

# this variable must bve defined or all options will not work 
if [ -z "$GEMPIS_STAT_HOME" ]; then
    echo "Error: variable GEMPIS_STAT_HOME is not defined. 
No statistics collection possible."
    exit 1
fi

# move into the directory and make report
cd $GEMPIS_STAT_HOME/$GEMPIS_RESOURCE 
export DURATION_LOG=`pwd`/seconds.log

# In most cases we don't need to login to a remote node because the
# data is also available on the current node.
case $2 in

collect)
	verbose "Option $2 detected."
	;;

resource)
	# can be done locally
	make_resource_report $3
	exit 0
	;;

*)
	usage
	exit 1
	;;
esac

#############################################################################
#
# If configuration indicates that the command has to go to a Head Node
# and I am not this computer, the pass the command along.
#

# indicate on what computer I am
verbose "Running on [$GEMPIS_HOSTNAME]."
if [ -n "$GEMPIS_HEADNODE_LIST" ]; then

    # indicate on what computer I am
    local_disk=`is_cwd_local_disk`
    verbose "Flag for running on local disk is [$local_disk]."
    [ -n "$local_disk" ] && terminate ERROR "The current working directory is on a local disk. 
    Data to be used on a remote computer must exist on a shared disk."

    # find a valid headnode
    find_headnode

    # am I not the headnode?
    if [ "$GEMPIS_HOSTNAME" != "`echo $GEMPIS_HEADNODE | awk -F'.' '{print $1}'`" ]; then
	send_gempis_command stat $*
	result=$?
	exit $result
    fi
fi

# the only reason to login to a remote node is to collect 
# run data 
collect_tmp_data

# terminate application
exit 0
