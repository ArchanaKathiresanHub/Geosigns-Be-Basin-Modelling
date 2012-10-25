#!/bin/csh
#
# C shell set up script for submitting elfen jobs
#
# See setup for more information
#

# define the base directory for the elfen projects work
setenv ELFEN_DIR /glb/eu/siep_bv/data/elfen

# get default version
set default_version = `cat $ELFEN_DIR/DEFAULT_VERSION | grep -v '#'`

cat <<EOF

csetup - Elfen parallel application set-up for csh.

Version $default_version is default. To use another version 
type:

    source csetup <version>

The following versions are available:
EOF
cd $ELFEN_DIR
ls -d elfendyn_* | sed 's/_/ /' | awk '{ printf( "%s ", $2) }'
echo ""
cd -

# Define a directory for project tools
# make sure that Gempis is defined in the right location (/apps/sssdev)
setenv PATH ${ELFEN_DIR}/bin:/apps/sssdev/share:$PATH

# setup for MPICH2
setenv MPICH2_HOME /nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/mpi/mpich2-1.0.3/LinuxRHEL64_x86_64_40AS
#setenv LD_LIBRARY_PATH $MPICH2_HOME/lib:$LD_LIBRARY_PATH
setenv PATH $MPICH2_HOME/bin:$PATH

# setup stuff for elven
if ( $#argv == 0 ) then

    # define the default version
    set version = $default_version
else
    set version = $argv[1]
endif

# define ELFEN Home and make sure it exists
setenv ELFENHOME ${ELFEN_DIR}/elfendyn_${version}
#setenv LD_LIBRARY_PATH ${ELFENHOME}/lib:${LD_LIBRARY_PATH}
setenv PATH ${ELFENHOME}/bin:${PATH}

cat <<EOF

To start an Elfen job type

    cd <project_directory>
    launch <#_processors> [<project_name>]

You can use the command 

    gempir Elfen

to view resources. See the 'Elfen - User Quick Ref' for details.

EOF
# End script
