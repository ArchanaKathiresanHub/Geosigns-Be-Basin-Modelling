#! /usr/bin/ksh
#
# This scripts creates a summary of LSF usage for a particular resource 
# for the previous month. An optional parameter can spcify which month 
# should be reported
# 
# Syntax: defined in a users crontab 
#0 0 1 * * /glb/home/nlgla8/SWEast/Management/lsf_stats.ksh
#

# define environment
. /etc/profile

# define resource name
resource=cldrn
BASEDIR=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Archive/Management

# check if month has been defined
month=$1
if [ -z "$month" ]; then
    month=`date '+%,' --date="1 month ago"`
fi
monthstr=`date '+%B_%Y' --date="$month/1"`

# LSF bhist format for start and end time
start=`date '+%Y/%m/01' --date="$month/01"`
end=`date '+%Y/%m/01' --date="$month/01 + 1 month"`

# check that we can run LSF command on this computer
which bhist >/dev/null 2>&1
if [ $? != 0 ]; then
    echo "This script must be run on an LSF node in order to work."
    exit 1
fi

# defin a base name for log files, etc.
script=`basename $0 | awk -F'.' '{ print $1 }'`

# list of user to receive mail
#mail_list="Gregory.Lackore@shell.com, Olivier.Meuric@shell.com, Sebastiaan.Keizer@shell.com, Brian.Campbell@shell.com, Peter.Huysse@shell.com"
mail_list="Gregory.Lackore@shell.com"

# user file
user_file=/apps/3rdparty/lsf_hpc6/conf/lsbatch/ams_shared_cluster/configdir/lsb.users

# location of LSF history files
#   currently stored based on file size, not date
LSFDIR=/apps/3rdparty/lsf_hpc6/work/ams_shared_cluster/logdir

# define output file locations
TMPFILE=`pwd`/tmp/$script.tmp
LOGFILE=$BASEDIR/log/$script-$monthstr.log
REPORT=$BASEDIR/reports/$script-$monthstr.txt

# list opf cauldron users
#user_list="nlgla8 nlwse0 ksafo3 ksphu0 nlamol nlima5 nldsc3"
#user_list="nlome0 nlqcl0 nlgya1 nlnya1 "
user_list=`grep cldrn $user_file | sed 's/(//' | sed 's/)//' | sed 's/cldrn//'`

# do a single user
do_user()
{
    user=$1

    [ -f $TMPFILE ] && rm $TMPFILE

    # for all defined files in the last week,
    for filename in `ls -lrt $LSFDIR/lsb.events* | awk '{print $9}'`; do

	# generate data for user during the period
	bhist -f $filename -P cldrn -u $user -S $start,$end -a -l | grep -v "No matching job found" >> $TMPFILE
    done

    total=`cat $TMPFILE | grep "^Job " | wc -l`

    if [ $total != 0 ]; then
	good=`cat $TMPFILE | grep "Done successfully." | wc -l`
	bad=`cat $TMPFILE | grep "Exited with exit code" | wc -l`
	abort=`cat $TMPFILE | grep "Exited by signal" | wc -l`
	unknown=`echo $total $good $bad $abort | awk '{ print $1 - $2 - $3 - $4 }'`

	printf "%8s %8d %8d %8d %8d %8d\n" $user $total $good $bad $abort $unknown
    fi

    # dump user information to log file
    cat $TMPFILE >> $LOGFILE
}

##############################################################################
# main
#

(
    # print title
    echo "LSF (monthly) job report for resource [$resource]" 
    echo "" 
    echo "    date `date`"
    echo ""
    echo "Reporting period: $monthstr"
    echo ""

    # print header
    printf "%8s %8s %8s %8s %8s %8s\n" UserId Total Success Failure Aborted Unknown 

    # for each user
    for user in $user_list; do

	do_user $user

    done

    echo ""
    echo ""
    echo "Data from this period has been stored in $LOGFILE for further investigation."
    echo ""
) | tee -a $REPORT

# send this output as mail to users
cat $REPORT | mail -s "LSF stats of resource $resource for $monthstr" $mail_list

# terminate application
exit 0
