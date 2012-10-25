#
# This is a wrapper for cfg2html (shortcut) and can be
# customized to fit your needs.
#
# Syntax: cfg2html <output_directory>
#
# the cfg2html programs create output using thr `hostname` as
# file prefix.
#
##############################################################

# get params
outdir="$1"
host=`hostname | awk -F'.' '{ print $1 }'`

# if parameter not given, print usage
if [ -z "$outdir" ]; then
	echo "Usage $0 <outdir>."
	exit 1
fi

# if variable not defined then use current
if [ -z "$GEMPIS_EXEDIR" ]; then
	echo "GEMPIS_EXEDIR not defined, will use `dirname $0`."
	export GEMPIS_EXEDIR=`dirname $0`
fi

##############################################################
#
# main
#

# determine script to run
case $(uname) in
SunOS) 	
	$GEMPIS_EXEDIR/cfg2html_sun -o $outdir -x
	;;
Linux)  
	$GEMPIS_EXEDIR/cfg2html_linux -o $outdir -x 
	;;
IRIX64)
	# this is a poor man's cfg2html
	/sbin/hinv -v -m > $outdir/$host.html
	;;
*)	echo "$0: Unsupported operating system!"; exit 2 ;;
esac
RETCODE=$?

# check for errors
if [ $RETCODE -ne 0 ]; then
    echo "Error ($0): Returncode=$RETCODE"
    exit 1
fi

# we don't want the text file, only err and HTML
#rm `hostname`*.txt* >/dev/null 2>&1

# return success
exit 0

# End script
