#
# script to launch elfen application
#
# Syntax: launch <#procs> [<project_name>]
#
# Dependencies:
# The elfen setup or csetup script defines the environment used here.
#

# define default resource
resource=Elfen
version="-v2008.04"

# get the number of processors and check
num_procs=$1
if [ -z "$num_procs" ]; then
    cat <<EOF

Usage: launch <num_procs> [<project>]

EOF
    exit 1
fi
[ -n "$2" ] && project=$2
    

if [ -z "$project" ]; then
    # the project name is determined by the CWD of the user.
    cwd=`pwd`
    suffix=`basename $cwd`
    project=`echo $suffix | awk -F'.' '{ print $1 }'`
    if [ -z "$project" ]; then
	cat <<EOF

Error! You must be IN the project directory before calling this command.

EOF
	exit 1
    fi
fi

# verify that project file exists
if [ ! -f ${project}.dat -a ! -f ${project}.neu ]; then
    echo "Error! Project file ${project}.dat and ${project}.neu not found"
    exit 1
fi

# Check that application is run on enough processors
let 'slaves = num_procs - 1'
if (( $slaves < 2 )); then
    cat <<EOF
Error! do not run parallel elfen with less than 2 slaves.

EOF
    exit 1
fi

# test that resource has been defined
gempis $version cfg resource $resource >/dev/null 2>&1
result=$?
if [ $result != 0 ]; then
    echo "Error! The resource [$resource] has not been defined."
    cat << EOF

See the 

           Elfen - User Quick Ref

for details.

EOF
    exit 2
fi

#
command=`which elfendyn`
echo "Launching elfendyn project [$project] with [$slaves] slaves."
echo "Calling [gempis $version $resource $num_procs $command $project]"
gempis $version $resource $num_procs $command $project
result=$?

echo "The result was [$result]."

# End script
