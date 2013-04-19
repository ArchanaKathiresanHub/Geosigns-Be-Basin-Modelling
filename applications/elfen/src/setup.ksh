#!/bin/bash
#
# Bash shell set up script for submitting elfen jobs
#
# Notes:
# The Elfen applications are statically linked so the LD_LIBRARY_PATH
# variable is not needed.
#

# define the base directory for the elfen projects work
export ELFEN_DIR=/glb/eu/siep_bv/data/elfen

echo ""
echo "setup - Elfen parallel application set-up for bash."
echo ""

# get default version
default_version=`cat $ELFEN_DIR/DEFAULT_VERSION | grep -v '#'`

if [ -z "$1" ]; then
    cat <<EOF
Version $default_version is default. To use another version type 
    . ./setup <version>

The following versions are available:
EOF
cd $ELFEN_DIR
ls -d elfendyn_* | sed 's/_/ /' | awk '{ printf( "%s ", $2) }'
echo ""
cd - >/dev/null
fi

# Define a directory for project tools
# and make sure that Gempis is defined in the right location (/apps/sssdev)
export PATH=${ELFEN_DIR}/bin:/apps/sssdev/share:$PATH

# setup for MPICH2
export MPICH2_HOME=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/mpi/mpich2-1.0.3/LinuxRHEL64_x86_64_40AS
#export LD_LIBRARY_PATH=$MPICH2_HOME/lib:$LD_LIBRARY_PATH
export PATH=$MPICH2_HOME/bin:$PATH

# setup stuff for elven
version=$1
if [ -z "$version" ]; then

    # define the default version
    version=$default_version
fi

# define ELFEN Home and make sure it exists
echo ""
echo "Setting up for Elfen version ${version}."
ELFENHOME=${ELFEN_DIR}/elfendyn_${version}
if [ ! -d ${ELFENHOME} ]; then
    echo "Error! Elfen version $version does not exist in $ELFEN_HOME."
    echo "Exitting..."
    exit 1
fi
export ELFENHOME
#export LD_LIBRARY_PATH=${ELFENHOME}/lib:${LD_LIBRARY_PATH}
export PATH=${ELFENHOME}/bin:${PATH}

cat <<EOF



To start an Elfen job type:
    cd <project_directory>
    launch <#_processors> [<project_name>]

You can use the command 
    gempir Elfen

to view resources. See the 'Elfen - User Quick Ref' for details.

EOF
# End script
