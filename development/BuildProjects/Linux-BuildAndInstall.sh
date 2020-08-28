#!/bin/bash
set

# Reading parameters with some defaults
svnRepository=file:///nfs/rvl/groups/ept-sg/SWEast/Cauldron/SUBVERSION/repository
src=${SRC_DIR:-`dirname $0`/../..}
platform=${PLATFORM:-Linux}
nprocs=${NUMBER_OF_CORES:-40}
deploy=${DEPLOY:-True}
geocase=${GEOCASE:-False}
version_number_major=${VERSION_NUMBER_MAJOR:-`date +%Y`}
version_number_minor=${VERSION_NUMBER_MINOR:-`date +%m`}
version_tag=${VERSION_TAG:-`whoami`}
configuration=${CONFIGURATION:-Release}

if [ "x$TMPDIR" != "x" ] && [ -e $TMPDIR ]; then
   tempdir=$TMPDIR/build_${configuration}
   mkdir -p ${tempdir}
else
   tempdir=`mktemp -d`
fi
build=${BUILD_DIR:-"${tempdir}"}

installdir=${INSTALL_DIR:-"${build}"}
unit_test_output=${UNIT_TEST_OUTPUT_DIR:-"${build}"}


cldgrp=`groups 2> /dev/null` || true
if [[ "$cldgrp" =~ "g_psaz00" ]];then
    cldgrp="g_psaz00"
else
    cldgrp=""
fi

# Set some code to execute when shell script exits
function onExit()
{
  # remove the temporary directory made for the build directory
  if [ "x${BUILD_DIR}" = x ]; then
     rm -Rf $build
  fi
}

trap onExit EXIT


# Test the build directory
test -d "$build" || { echo "Given build directory '$build' does not exist"; exit 1; }
ls "$build"/* > /dev/null 2>&1 && { echo "Given build directory '$build' is not empty"; exit 1; }


# set UMASK
umask 0002

# set dirs as absolute paths
pushd $src
src="`pwd -P`"
popd

pushd $build
build="`pwd -P`"
popd

######################
# Standard applications
if [ -e /glb/home/ksaho3/bin.Linux/svn ];
then
    SVN=/glb/home/ksaho3/bin.Linux/svn
else
    SVN=`which svn`
fi

if [ -e /nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/bin/cmake ];
then
    CMAKE=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/bin/cmake
else
    CMAKE=`which cmake`
fi
if [ -e /nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/bin/ctest ];
then
    CTEST=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/bin/ctest
else
    CTEST=`which ctest`
fi

# Load module for intel 2017.05 compiler
set +x
. /glb/apps/hpc/Lmod/etc/profile.d/z01_lmod-hpcs.sh
module load HpcSoftwareStack/PRODUCTION
module purge
module load intel/2019a
set -x

# Set the simplest locale so that there won't be any text conversion problems
# for the logged output between Linux and Windows
export LANG=C

# Build applications
echo Building Cauldron applications
pushd $build
${src}/development/bootstrap.sh \
      -DCMAKE_BUILD_TYPE=${configuration} \
      -DBM_VERSION_NUMBER_MAJOR=${version_number_major} \
      -DBM_VERSION_NUMBER_MINOR=${version_number_minor} \
      -DBM_VERSION_TAG=${version_tag} \
      -DBM_UNIT_TEST_OUTPUT_DIR=${unit_test_output} \
      -DCMAKE_INSTALL_PREFIX=${installdir} \
      "$@" \
  || { echo "CMake : error : Configuration has failed"; exit 1; } 

source envsetup.sh

# Make piped commands fail if any of the subcommands fail... This is needed
# shortly after
set -o pipefail

# Run Gnu Make :-)
# Notes on parameters:
#
#   -k  => build as much as you can
#
#   -j  => parallel build
#
#   3>&1 1>&2 2>&3 3>&-  
#       => Swap stdout and stderr, so that subsequent sed scripts work on the stderr.
#
#   sed -e 's/:\(.*\): \([Ww]arning\|[Ee]rror\): /(\1): \2 : /' | \
#   sed -e 's/: \(undefined reference\) /: error : \1/' 
#       => Make GCC and LD outputted warnings easier to digest for TFS / Visual Studio
#          (See also http://stackoverflow.com/questions/3441452/msbuild-and-ignorestandarderrorwarningformat)
#
make -k -j${nprocs} \
     3>&1 1>&2 2>&3 3>&-  \
     | sed -e 's/:\(.*\): \([Ww]arning\|[Ee]rror\): /(\1): \2 : /'  \
     | sed -e 's/: \(undefined reference\) /: error : \1 /' \
         || { echo "GNU Make : error : Build has failed "; exit 1 ; } 

if [ x$deploy = xTrue ]; then
   if [[ ${configuration} =~ "[Rr]elease" ]]; then
      make install/strip || { echo "GNU Make : error : Installation has failed"; exit 1 ; }
   else
      make install || { echo "GNU Make : error : Installation has failed"; exit 1 ; } 
   fi
fi

#Give access for the group g_psaz00 to build folder
if [ "$cldgrp" ]; then
  chgrp -R $cldgrp $build
fi
chmod -R g+rw $build
find $build -type d -print0 | xargs -0 chmod g+x

if [ x$geocase = xTrue ]; then
# Import (legacy) Geocase
   echo Building Cauldron Geocase applications
   rm -rf $src/development/geocase/BB $src/development/geocase/misc/GospelScriptFiles
   ${SVN} export -q ${svnRepository}/trunk/development/geocase/BB $src/development/geocase/BB
   ${SVN} export -q ${svnRepository}/trunk/development/geocase/misc/GospelScriptFiles $src/development/geocase/misc/GospelScriptFiles

# Build Geocase applications
   rm -rf geocase_build
   mkdir -p geocase_build
   pushd geocase_build
   ${CMAKE} ${src}/development/geocase/BB_Lists \
       -DCMAKE_INSTALL_PREFIX=${installdir} \
       -DCMAKE_BUILD_TYPE=${configuration}
   make -j${nprocs}

   if [ x$deploy = xTrue ]; then
      make install/strip
   fi

   popd
else
   echo Geocase applications have been excluded
fi

# Install to SSSDEV
if [ x$deploy = xTrue ]; then
   echo "Install fresh binaries"
   if bash InstallAll.sh ; then
     echo "Installation complete at the first attempt"
   else
     echo "Let's try to rotate the installation first..."
     bash MoveInstalls.sh || { echo "MoveInstalls.sh : error : Deployment has failed"; exit 1 ; }
     echo "Again, trying to install fresh binaries..."
     bash InstallAll.sh || { echo "InstallAll.sh : error : Deployment has failed"; exit 1 ; }
     echo "Installation succeeded at the second attempy."
   fi 
 
   echo 
   echo "==============================================================="
   echo "IBS/Cauldron v${version_number_major}.${version_number_minor}${version_tag} has been installed succefully on /apps/sssdev"
   echo "==============================================================="
   echo
else
   echo Application deployement has not been requested.
fi 
  
# Run Unit Tests
${CTEST} || { echo "CTest : error: One or more unit tests have failed"; exit 1 ; } 

popd
