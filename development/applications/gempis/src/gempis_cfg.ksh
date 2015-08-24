#
# gempis_cfg - GEneric MPIrun Submission Configuration tool
#
#############################################################################


# check that this routine has been called in the correct wat
if [ -z "$GEMPIS_USERNAME" ]; then
    echo "Error! Invalid command format, please use 

    gempis cfg

instead.
"
    exit 1
fi

# name use by CSCE to locate this program
export GEMPIS_PROGNAME="gempis_cfg"
verbose "$GEMPIS_PROGNAME [$*]."

############################################################################
#
# tell user how I work and exit
#
usage()
{
    # terminate application 
    terminate ERROR "Invalid command line. The options are:

    gempis cfg help
               computer
               defs
               jobtype [<jobtype>]
               mpiref [<mpirfe>]
               resource [<resource>]
               export
               import
"
    exit 1
}

############################################################################
#
# calls cfg2html, any special checks can be done here
#
# cfg2html generates it own file called `hostname`
do_cfg2html()
{
    # no background images
    $GEMPIS_EXEDIR/cfg2html $GTMPDIR

    # remove unnecessary stuff
    rm -f $GTMPDIR/*.txt
    rm -f $GTMPDIR/*.save
}


############################################################################
#
# get information about the user
#
get_user()
{
    echo "" 
    echo "Collecting user information." 

    date

    echo "---- User name is: ---"
    echo "$GEMPIS_USERNAME"

    echo "---- Id(s) are: ---"
    id

    echo "---- Finger says: ---"
    finger $GEMPIS_USERNAME

    echo "---- Groups are: ---"
    groups 

    echo "---- Quotas are: ---"
    quota
}


############################################################################
#
# packs information generated and sends it off
#
pack_data()
{
    # pack up data and send if off to me
    echo ""
    echo "Bundling data in `pwd`."

    # create compressed tarball of everything in the tmp directory
    echo "Executing command [tar cf $OUTFILE $GTMPDIR]."
    tar cf $OUTFILE $GTMPDIR

    echo "Executing command [gzip -f $OUTFILE]."
    gzip -f $OUTFILE

    # tell user what's happening
    echo ""
    echo "Configuration results are now saved in "
    echo ""
    echo "    $OUTFILE.gz."
    echo ""
}

############################################################################
#
#
#
make_userdir()
{
    mydir=$1
    if [ -z "$mydir" ]; then
	echo "No directory name"
	exit 1
    fi

    if [ ! -d $mydir ]; then
	echo "Creating temporary config directory $mydir."
	mkdir -p $mydir
	chmod 777 $mydir
    fi

    if [ ! -d $mydir ]; then
	echo "Error. Could not create directory $mydir."
	echo ""
	echo "Aborting procedure."
	echo ""
	exit 1
    fi

    # clear out any previous data
    rm -f $mydir/* >/dev/null 2>&1

    sleep 2
}


############################################################################
#
#
#
get_computer()
{
    echo ""
    echo "This procedure extracts user and computer information, then"
    echo "   sends the result to the GEMPIS administrator."
    echo ""

    # make temp directory
    make_userdir $GTMPDIR

    # get computer information 
    do_cfg2html

    # get information about the user
    get_user > $GTMPDIR/$GEMPIS_USERNAME.txt

    # pack the data up 
    pack_data

    # sen data to helpdesk
    . $GEMPIS_EXEDIR/gempis_adm email $FILENAME.gz
    result=$?
    echo ""

    if [ $result != 0 ]; then
	echo "Mail returned with a failure (error $result). Please send the file "
	echo ""
	echo "    $OUTFILE.gz"
	echo ""
	echo "by some other means to $GEMPIS_EMAIL_ADMINISTRATOR."
    else
	echo "The file"
	echo ""
	echo "    $OUTFILE.gz"
	echo ""
	echo "has been accepted by your workstation's mail program. In the event "
	echo "that the daemon is unable to send this file, please use another "
	echo "form of e-mail and send this file to $GEMPIS_EMAIL_ADMINISTRATOR."
    fi
    echo ""
    echo "Procedure complete."
    echo ""
}


############################################################################
#
# show the defined jobtypes
#
show_definitions()
{
    echo "Gempis environment definitions"
    echo ""
    env | grep GEMPIS_ | sort 
    echo ""
}


############################################################################
#
# show the defined jobtypes
#
list_jobtypes()
{
    # get system jobtype
    files="`ls $GEMPIS_DEFDIR/jobtype_* | awk -F'_' '{ print $2 }'`"
    verbose "System jobtypes are [$files]."

    # if user jobtypes exist, get them
    ls $GEMPIS_USERDIR/jobtype_* >/dev/null 2>&1
    if [ $? = 0 ]; then
	files="$files `ls $GEMPIS_USERDIR/jobtype_* | awk -F'_' '{ print $2 }'`"
    fi
    verbose "User defined jobtypes are [$files]."

    list=""
    for file in $files; do
       list="$list `basename $file`"
    done

    # remove first space
    list=`echo "$list" | sed 's/ //'`
    echo "$list"
}


############################################################################
#
# show the defined jobtypes
#
show_jobtype()
{
    GEMPIS_JOBTYPE=$1

    echo "GEMPIS Job Type information"
    echo ""
    echo "Job type name:  $GEMPIS_JOBTYPE"
    echo ""

    # determin if the name is in the known list
    found="no"
    for file in `list_jobtypes`; do
	if [ "$file" = "${GEMPIS_JOBTYPE}" ]; then
	    found=yes
	    break
	fi
    done
    if [ $found = "no" ]; then
	echo "ERROR! Jobtype $GEMPIS_JOBTYPE not found."
	exit 1
    fi

    # verify that the script can be found
    verify_exec jobtype_${GEMPIS_JOBTYPE}

    # tell where the script can be found
    echo "This script will be executed from:"
    which jobtype_${GEMPIS_JOBTYPE}
}

############################################################################
#
# show the defined MPI references
# 
#
list_mpirefs()
{
    echo "${mpi_name[*]}"
}


############################################################################
#
# show the defined MPI references
# 
show_mpiref()
{
    GEMPIS_MPIREF=$1

    define_mpiref
    echo "GEMPIS MPI Reference information"
    echo ""

    echo "MPI Reference name:  $GEMPIS_MPIREF"
    if [ $GEMPIS_MPI_INDEX -lt 11 ]; then
	echo "    This is a system resource."
    else
	echo "    This is a user defined resource."
    fi

    # display the configuration
    echo ""
    echo "MPI index:           $GEMPIS_MPI_INDEX"
    echo "Maximum index:      [$GEMPIS_MAXMPIREF]"
    echo "Target O/S:         [$GEMPIS_MPI_OS]"
    echo "Executable PATH:    [$GEMPIS_MPI_PATH]"
    echo "Run command:        [$GEMPIS_MPI_RUN]"
    echo "Library PATH:       [$GEMPIS_MPI_LIB]"
    echo "Start command:      [$GEMPIS_MPI_START]"
    echo "Stop command:       [$GEMPIS_MPI_STOP]"
    echo "Build host file:"
    typeset -f mpi_buildfile_${GEMPIS_MPIREF} | awk '{ printf("                    %s\n", $0) }'
    echo "Prejob subroutine:"
    typeset -f mpi_prejob_${GEMPIS_MPIREF} | awk '{ printf("                    %s\n", $0) }'
    echo "Postjob subroutine:"
    typeset -f mpi_postjob_${GEMPIS_MPIREF} | awk '{ printf("                    %s\n", $0) }'
    echo ""
}


############################################################################
#
# show the available resources

# 
# Names in the format xxx.`uname` are filtered out, so that only xxx remains.
#
list_resources()
{
    # show defined resources
    verbose "Complete resource list is [${res_name[*]}]."

    list=""
    for i in ${res_name[*]}; do
       entry="`echo $i | awk -F'.' '{ print $1 }'`"
       skip="NO"
       for j in $list; do
	   [ $entry = $j ] && skip=YES
       done
       [ $skip = "NO" ] && list="$list $entry"
    done

    # remove first space
    list=`echo "$list" | sed 's/ //'`
    echo "$list"
}


############################################################################
#
# show how the resource is configured
#
show_resource()
{
    echo "GEMPIS resource information"
    echo ""

    echo "Resource name:  $GEMPIS_RESOURCE"
    if [ $GEMPIS_RES_INDEX -lt 11 ]; then
	echo "    This is a system resource."
    elif [ $GEMPIS_RES_INDEX -lt 21 ]; then
	echo "    This is a regional resource."
    else
	echo "    This is a user defined resource."
    fi

    echo ""
    echo "Resource index: $GEMPIS_RES_INDEX"
    echo "Maximum index:  [$GEMPIS_MAXRESOURCE]"
    echo "Job Type:       [$GEMPIS_JOBTYPE]"
    echo "MPI Reference:  [$GEMPIS_MPIREF]"
    echo "Headnode list:  [$GEMPIS_HEADNODE_LIST]"
    echo "Request:        [$GEMPIS_REQUEST]"
    echo "Submit:         [$GEMPIS_SUBMIT]"
    echo "Prepath:        [$GEMPIS_PREPATH]"
    echo "Cluster:        [$GEMPIS_CLUSTER]"
    echo ""
}

############################################################################
#
# tar gempis configuration and send it to support
#
export_configuration()
{
    tarball=`whoami`.tar.gz
    echo ""
    echo "Export user Gempis configuration to file $tarball."

    cd 
    tar cfz $tarball .gempis
    if [ $? != 0 ]; then

	echo ""
	echo "Export failed!"
	exit 1
    else
	echo ""
	echo "Export was successful."
    fi

    echo "E-mailing Gempis configuration to $GEMPIS_EMAIL_HELPDESK."
    . $GEMPIS_EXEDIR/gempis_adm email $tarball

    # return to previous directory
    cd - >/dev/null
}


############################################################################
#
# tar gempis configuration and send it to support
#
import_configuration()
{
    tarball=gempis_import.tar.gz

    echo ""
    echo "Importing user Gempis configuration from file $tarball."

    cd

    if [ ! -f $tarball ]; then
	echo "Error! import file $tarball not found in HOME directory."
	exit 1
    fi

    echo ""
    echo "Extracting data."
    tar xfz $tarball
    if [ $? != 0 ]; then

	echo ""
	echo "Import failed!"

	exit 1
    else
	echo ""
	echo "Import was successful."
    fi

    # return to previous directory
    cd - >/dev/null
}


############################################################################
#
# main
#

# if no parameters then display usage message
[ $# -lt 1 ] && usage

# load only system environment settings
load_definitions skip

# define interrupt handler
verbose "Defining gempis_cfg trap handler."
trap trap_handler INT TERM QUIT HUP

# define cfg stuff
GTMPDIR=$GEMPIS_PROGNAME
FILENAME=${GEMPIS_PROGNAME}-${GEMPIS_USERNAME}.tar
OUTFILE=`pwd`/$FILENAME

# if told to send the data, send it
if [ "$1" = "help" ]; then
    show_help
    exit 0
fi
if [ "$1" = "computer" ]; then

    get_computer
    exit 0
fi

# 
# at this point it is assumed that gempis has been setup
# and is working. The following routine will show various 
# configuration settings.
#

# load system and user environment settings
load_definitions

if [ "$1" = "defs" ]; then
    show_definitions
    exit 0
fi

if [ "$1" = "export" ]; then
    export_configuration
    exit 0
fi
if [ "$1" = "import" ]; then
    import_configuration
    exit 0
fi


# load resource definitions
load_resources

if [ "$1" = "jobtype" ]; then
    if [ -z "$2" ]; then
	list_jobtypes
    else
	show_jobtype $2
    fi
    exit 0
fi

if [ "$1" = "mpiref" ]; then
    define_resource LOCAL

    # define the environment variables so they will be seen
    GEMPIS_MPISTART_OPTIONS='$GEMPIS_MPISTART_OPTIONS'
    GEMPIS_MPIRUN_OPTIONS='$GEMPIS_MPIRUN_OPTIONS'
    GEMPIS_EXEDIR='$GEMPIS_EXEDIR'
    GEMPIS_NUMCPUS='$GEMPIS_NUMCPUS'
    GEMPIS_MACHINEFILE='$GEMPIS_MACHINEFILE'

    load_mpirefs

    if [ -z "$2" ]; then
	list_mpirefs
    else
	show_mpiref $2
    fi
    exit 0
fi

if [ "$1" = "resource" ]; then
    # define resource name
    if [ -z "$2" ]; then
	list_resources
    else
	define_resource $2
	show_resource
    fi
    exit 0
fi

echo "Unknown option [$1]"
usage

exit 1
# End script
