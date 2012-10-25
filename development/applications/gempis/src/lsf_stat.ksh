#! /bin/ksh
#
# This scripts creates a summary of LSF usage for a particular resource 
# for the previous month. An optional parameter can spcify which month 
# should be reported
# 
# Syntax: defined in a users crontab 
# lsf_stats.ksh [yyyy-mm]
#

#############################################################################
#
# Setup CSCE if not defined

# name use by CSCE to locate this program
export PROGNAME="gempis_stat"

#############################################################################
# 
# This definition is used througout gempis to locate gempis executables
#
export GEMPIS_PROGNAME=$PROGNAME

# check that required variable is set
#[ -z "$IBS_NAMES" ] && terminate ERROR "IBS_NAMES environment variable not defined."
# This definition is used througout gempis to locate gempis system definitions
#export GEMPIS_DEFDIR="$IBS_NAMES/gempis"
#export GEMPIS_SYSTEM_DEF="$GEMPIS_DEFDIR/definitions_system"

############################################################################
#
# interrupt handler subroutine, when INT has been pressed
#
trap_handler()
{
    # terminate application
    echo "trap handler"
}

############################################################################
#
# tell user how I work and exit
#
usage()
{
    # terminate application 
    echo "Usage $0 <yyyy> <mm>"
}


############################################################################
#
# main
#

# define the system and user environment settings
#load_definitions

# if no parameters then display usage message
[ $# -lt 1 ] && usage

# load resources
#load_resources ignore

# process some of the simple commands
if [ "$1" = "help" ]; then
    show_help
    exit 0
fi

#usage


############################################################################
#
# If we are here then the resource name has been defined
#

# define interrupt handler
trap trap_handler 2 15

# define resource
#define_resource $1

# setup definitions for the jobtype
#load_jobtype

#########################################################################
# 
# this is an example of what the routine should become


# define environment
#. /etc/profile


# define resource name
#BASEDIR=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Archive/Management
BASEDIR=/glb/home/nlgla8/CBM_Statistics

# check if month has been defined
year=$1
if [ -z "$year" ]; then
	year=2008
fi
month=$2
if [ -z "$month" ]; then
    month=`date '+%m' --date="1 month ago"`
fi
monthstr=`date '+%Y-%m' --date="$year/$month/1"`

# LSF bhist format for start and end time
start=`date '+%Y/%m/01' --date="$year/$month/01"`
end=`date '+%Y/%m/01' --date="$year/$month/01 + 1 month"`

# check that we can run LSF command on this computer
which bhist >/dev/null 2>&1
if [ $? != 0 ]; then
    echo "This script must be run on an LSF node in order to work."
    exit 1
fi

# defin a base name for log files, etc.
script=`basename $0 | awk -F'.' '{ print $1 }'`

# list of user to receive mail
#mail_list="Gregory.Lackore@shell.com, Olivier.Meuric@shell.com, Sebastiaan.Keizer@shell.com"
mail_list="Gregory.Lackore@shell.com"

host=`hostname`
case $host in
amsdc1-n-s00037)
  # Darwin Amsterdam
  #   currently stored based on file size, not date
  cluster=Darwin
  resource=cldrn
  LSFDIR=/apps/3rdparty/lsf_hpc6/work/ams_shared_cluster/logdir
  user_file=/apps/3rdparty/lsf_hpc6/conf/lsbatch/ams_shared_cluster/configdir/lsb.users
  #user_list="nlome0 nlqcl0 nlgya1 nlnya1 "
  #user_list="nlgla8 nlwse0 ksafo3 ksphu0 nlamol nlima5 nldsc3"
  user_list=`grep $resource $user_file | sed 's/(//' | sed 's/)//' | sed 's/cldrn//'`
  ;;

amsdc1-n-s01taw)
  cluster=EPDTW-AMS
  resource=CAULDRON
  LSFDIR=/apps/3rdparty/lsf/work/EPDTW_AMS/logdir
  #user_file=/apps/3rdparty/lsf/conf/lsbatch/EPDTW_AMS/configdir/lsb.users
  user_list="gbele0 nlcke5 gbsst1 nlgya1 nlgla8"
  ;;

houic-n-s00034)
  # Houston
  # user file does not have names
  cluster=Galileo
  resource=cldrn
  LSFDIR=/apps/3rdparty/lsf_hpc6/work/GALILEO/logdir
  user_list="nlaju1 nlgla8 nlnya1 uscbu1 usdbae usdra4 uskle2"
  ;;

pth-n-s01taw)
  #Perth
  # user file does not have names
  cluster=EPDTW-SDA
  resource=EPDTW
  LSFDIR=/apps/3rdparty/lsf/work/EPDTW/logdir
  user_list="aubma6 nlble2 nlmhah uspkr0 nlgla8"
  ;;

*)
  echo "ERROR! Unknown region on [$host]."
  exit 1
  ;;
esac

# location of LSF history files
#   currently stored based on file size, not date

# define output file locations
TMPFILE=`pwd`/tmp/$script.tmp
LOGFILE=$BASEDIR/log/$script-$monthstr.log
REPORT=$BASEDIR/reports/$script-$monthstr.txt

[ ! -d `pwd`/tmp ] && ( mkdir `pwd`/tmp; chmod 777 `pwd`/tmp )
[ ! -d $BASEDIR/log ] && ( mkdir $BASEDIR/log; chmod 777 $BASEDIR/log )
[ ! -d $BASEDIR/reports ] && ( mkdir $BASEDIR/reports; chmod 777 $BASEDIR/reports )

# do a single user
do_user()
{
    user=$1
    [ -n "$GEMPIS_VERBOSE" ] && echo "Checking for user $user in $start,$end." 

    [ -f $TMPFILE ] && rm $TMPFILE

    # for all defined files in the last week,
    for filename in `ls -lrt $LSFDIR/lsb.events* | awk '{print $9}'`; do

	[ -n "$GEMPIS_VERBOSE" ] && echo " Parsing file $filename" 

	# generate data for user during the period
	#bhist -f $filename -P cldrn -u $user -S $start,$end -a -l | grep -v "No matching job found" >> $TMPFILE
	bhist -f $filename -u $user -S $start,$end -a -l | grep -v "No matching job found" >> $TMPFILE
    done

# Example output
#Job <6606>, Job Name <BrowseExtension:fastmig>, User <nlble2>, Project <default
#                     >, Interactive mode, Command </apps/sss/ibs/v2007.05/Linux
#                     RHEL64_30AS/bin/gempis_mpiwrapper>
#Sat Aug  4 19:16:04: Submitted from host <pth-n-s00017>, to Queue <epdtw_cauldr
#                     on>, CWD </glb/ap/sda/data/cauldron/BROWSE_GDA94/cauldroni
#                     nput>, 32 Processors Requested;
#Sat Aug  4 19:16:08: Dispatched to 32 Hosts/Processors <4*pth-n-s00007> <4*pth-
#                     n-s00018> <4*pth-n-s00014> <4*pth-n-s00006> <4*pth-n-s0001
#                     9> <4*pth-n-s00017> <4*pth-n-s00016> <4*pth-n-s00005>;
#Sat Aug  4 19:16:08: Starting (Pid 11422);
#Sat Aug  4 19:47:45: Done successfully. The CPU time used is 206.2 seconds;
#Sat Aug  4 19:47:45: Post job process done successfully;
#
#Summary of time in seconds spent in various states by  Sat Aug  4 19:47:45
#  PEND     PSUSP    RUN      USUSP    SSUSP    UNKWN    TOTAL
#  4        0        1897     0        0        0        1901
#------------------------------------------------------------------------------
    # LSF can forget that a job has terminated, the time it calculates is then
    # from the time it starts until the present moment.

    total=`cat $TMPFILE | grep "^Job " | wc -l`
    if [ $total != 0 ]; then
	good=`cat $TMPFILE | grep "Done successfully." | wc -l`
	bad=`cat $TMPFILE | grep "Exited with exit code" | wc -l`
	abort=`cat $TMPFILE | grep "Exited by signal" | wc -l`
	unknown=`echo $total $good $bad $abort | awk '{ print $1 - $2 - $3 - $4 }'`
	hours=`cat $TMPFILE | awk 'BEGIN { 
  header_found = 0; 
  job_completed = 0;
  seconds = 0 
}
{
  if ( $1 == "Job" )
  {
    header_found = 0;
    job_completed = 0;
  }
  if ( header_found == 1 )
  {
    /* job completeion detected */
    if ( job_completed == 1 )
    {
        seconds += ( $3 * procs );
    }
    header_found = 0;
    job_completed = 0;
    /* printf( "total is %d\n", seconds );*/
  }
  if ( $5 == "Done" || $5 == "Exited" )
  {
    job_completed = 1;
  }
  if ( $3 == "RUN" )
  {
    /* printf( "header_found set\n"); */
    header_found = 1; 
  }
  if ( $5 == "Dispatched" )
  {
    procs = $7; 
    /* printf( "[%s] procs to %d\n", $0, procs); */
  }
}
END {
       printf( "%d\n", ( seconds + 1800 ) / 3600 )
}'`

	printf "%8s %8d %8d %8d %8d %8d %8d\n" $user $hours $good $bad $abort $unknown $total 
    fi

    # dump user information to log file
    cat $TMPFILE >> $LOGFILE
}

##############################################################################
# main
#

(
    # print title
    echo "LSF (monthly) job report for cluster [$cluster]" 
    echo "" 
    echo "    date `date`"
    echo ""
    echo "Reporting period: [$monthstr]"
    echo ""

    # print header
    printf "%8s %8s %8s %8s %8s %8s %8s\n" UserId Hours Success Failure Aborted Unknown Total  

    # for each user
    for user in $user_list; do

	do_user $user

    done | tee summary.log

    # create summary
    echo "----------------------------------------------------------------------------"
    cat summary.log | awk '{
    hours += $2 
    suc += $3
    fail += $4 
    abort += $5
    unknown += $6
    tot += $7
}
END {
    printf( "%-8s %8s %8s %8s %8s %8s %8s\n", "Total", hours, suc, fail, abort, unknown, tot );
}'

    # add trailer
    cat <<EOF 

Data from this period has been stored in 

  $REPORT

for further investigation.

EOF

) | tee $REPORT

# send this output as mail to users
cat $REPORT | mail -s "$cluster LSF stats for $monthstr" $mail_list

# terminate application
exit 0
