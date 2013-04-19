#
# Syntax: mon <resource> <jobid>
#
# filename - list of files to monitor
# jobid    - LSF job id
#
# create in the following format
# row 1 (top) CPU states
# row 2 (middle) Memory usage
# row 3 (bottom) Network I/O
#
################################################################################

# check that this routine has been called in the correct wat
if [ -z "$GEMPIS_USERNAME" ]; then
    echo "Error! Invalid command format, please use 

    gempis mon

instead.
"
    exit 1
fi

# name use to locate help page misc/HTML
# set to gempis so it also uses the special format messages
export GEMPIS_PROGNAME="gempis_mon"
verbose "gempis_mon [$*]."


############################################################################
#
# tell user how I work and exit
#
usage()
{
    # terminate application 
    terminate ERROR "Invalid command line. The options are:

    gempis mon help
               <resource> <jobid>
"
}


###############################################################################
#
# params:
# $1   - remote host where this script is started
screenOwner () 
{
    host=$1
    xuser=`ssh -x -q -o 'StrictHostKeyChecking=no' $host 'who' | awk '{print $1" "$2}' | grep ':0' | awk '{print $1}'`

    [ -n "$xuser" ] && xuser=`finger $xuser | grep Name | sed '{s/.*: *//;s/ /_/g;}'`

    echo ${host}:${xuser:-NoBody}
}


###############################################################################
#
# parses for all hosts
#
screenOwners () 
{
    for host in $*; do
	screenOwner $host &
    done
    wait
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

        # try to enable display for use by other computers
	xhost + >/dev/null

	# forward command to headnode
	send_gempis_command mon $*
	exit $?
    fi
fi

#############################################################################
#
# if here the run monitor
#

# get rid of resource name
shift

# define trap handlers
#verbose "Defining monitor trap handler."
#trap mon_trap_handler INT TERM QUIT HUP

# worksheet configuration file name (default)
f=`pwd`/ksysguard.sgrd
infile=${1:-all.hosts}
if [ -f $infile ]; then

    host_list=`(cat $infile) | sort -u`
    nhosts=`echo "$host_list" | wc -l`
else

    allhosts=`bjobs -w $infile | tail -n +2 | awk '{ print $6 }'| sed 's/\*/ /g'`
    host_list=`echo "$allhosts" | sed 's/^[1-9]//' | sed 's/:.//g'`
    cpu_list=`echo "$allhosts" | sed 's/:/ /g' | awk '{ for ( i=1; i<NF; i+=2 ) printf( "%d ", $i ) }'`
    nhosts=`echo $host_list | wc -w`
fi

# no information 
if [ "$nhosts" = 0 ]; then
    echo "Error!"
    echo "No information found for job id $infile."
    echo ""
    echo "Terminating."
    exit 1
fi

# list of unique hosts with user
screenowners=`screenOwners $host_list`
MAXCOL=${MAXCOL:-$nhosts}
NROWS=$((3*((nhosts-1)/MAXCOL+1)))

#
# start building the configuration worksheet file
#
cat << EOF > $f
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE KSysGuardWorkSheet>
<WorkSheet rows="${NROWS}" interval="2" columns="$MAXCOL" >
EOF

# define entry for host
for host in $host_list; do
    cat << EOF >> $f
 <host port="-1" shell="" name="$host" command="ssh $host ksysguardd" />
EOF
done

#
#
nrows=3
row0=0
row1=1
row2=2
col=0
i=0
while true; do

    let 'i = i + 1'

    host=`echo $host_list | awk -v i=$i '{ print $i }' | awk -F'.' '{ print $1 }'`
    if [ -z "$host" ]; then
	break
    fi

    cpus=`echo $cpu_list | awk -v i=$i '{ print $i }'`

    # define display for CPU
    cat << EOF >> $f
 <display topBar="0" vColor="326429" title="${host}: CPU (* ${cpus})" bColor="3223601" graphStyle="0" class="FancyPlotter" row="$row0" unit="" hScale="1" column="$col" showUnit="0" hLines="1" hCount="5" vLines="1" autoRange="0" min="0" max="100" hColor="326429" globalUpdate="1" pause="0" fontSize="9" labels="1" vScroll="1" vDistance="30" >
  <beam sensorName="cpu/user" hostName="$host" color="1608191"  sensorType="integer" />
  <beam sensorName="cpu/sys"  hostName="$host" color="16743688" sensorType="integer" />
  <beam sensorName="cpu/nice" hostName="$host" color="16771600" sensorType="integer" />
 </display>
EOF

    # define Memory usage display
# I believe this i9s a combination of the other 
#  <beam sensorName="mem/physical/used" hostName="$host" color="16767744" sensorType="integer" />

    cat << EOF >> $f
 <display topBar="0" vColor="326429" title="Memory" bColor="3223601" graphStyle="0" class="FancyPlotter" row="$row1" unit="" hScale="1" column="$col" showUnit="0" hLines="1" hCount="5" vLines="1" autoRange="1" min="0" max="0" hColor="326429" globalUpdate="1" pause="0" fontSize="9" labels="1" vScroll="1" vDistance="30" >
  <beam sensorName="mem/physical/application" hostName="$host" color="37631" sensorType="integer" />
  <beam sensorName="mem/physical/buf" hostName="$host" color="16711680" sensorType="integer" />
  <beam sensorName="mem/physical/cached" hostName="$host" color="16767744" sensorType="integer" />
  <beam sensorName="mem/physical/free" hostName="$host" color="46592" sensorType="integer" />
 </display>
EOF

    # define network display
    cat << EOF >> $f
 <display topBar="0" vColor="326429" title="Network" bColor="3223601" graphStyle="0" class="FancyPlotter" row="$row2" unit="" hScale="1" column="$col" showUnit="0" hLines="1" hCount="5" vLines="1" autoRange="1" min="0" max="0" hColor="326429" globalUpdate="1" pause="0" fontSize="9" labels="1" vScroll="1" vDistance="30" >
  <beam sensorName="network/interfaces/eth0/receiver/data" hostName="$host" color="16711680" sensorType="integer" />
  <beam sensorName="network/interfaces/eth0/transmitter/data" hostName="$host" color="1608191" sensorType="integer" />
 </display>
EOF

#  <beam sensorName="network/interfaces/eth0/transmitter/packets" hostName="$host" color="1608191" sensorType="integer" />
#  <beam sensorName="network/interfaces/eth0/receiver/packets" hostName="$host" color="16771860" sensorType="integer" />

    # make the rest empty
    col=$((col+1))
    if [ $col -ge $MAXCOL ]; then
	col=0
	row0=$((row0+nrows))
	row1=$((row1+nrows))
	row2=$((row2+nrows))
    fi
done

#
cat <<EOF >> $f
</WorkSheet>
EOF

# make sure that LD_LIBRARY_PATH does not have a Qt library defined
#   otherwise it generates alot of '&' in the windows created
export LD_LIBRARY_PATH=

# call command to do display
ksysguard $f

# End script
