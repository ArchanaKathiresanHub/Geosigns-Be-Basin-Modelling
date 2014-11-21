#!/bin/bash

# Reading parameters with some defaults
svnRepository=file:///nfs/rvl/groups/ept-sg/SWEast/Cauldron/SUBVERSION/repository
src=${SRC_DIR:-`dirname $0`/../..}
build=${BUILD_DIR:-`mktemp -d`}
installdir=${INSTALL_DIR:-"${build}"}
unit_test_output=${UNIT_TEST_OUTPUT_DIR:-"${build}"}
platform=${PLATFORM:-Linux}
configuration=${CONFIGURATION:-Release}
nprocs=${NUMBER_OF_CORES:-20}
deploy=${DEPLOY:-True}
geocase=${GEOCASE:-True}
version_number_major=${VERSION_NUMBER_MAJOR:-`date +%Y`}  
version_number_minor=${VERSION_NUMBER_MINOR:-`date +%m`}  
version_tag=${VERSION_TAG:-`whoami`}

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

# Standard applications
SVN=/glb/home/ksaho3/bin.Linux/svn
CMAKE=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/cmake/cmake-2.8.10.2/Linux64x_26/bin/cmake
CTEST=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/cmake/cmake-2.8.10.2/Linux64x_26/bin/ctest

# Set the simplest locale so that there won't be any text conversion problems
# for the logged output between Linux and Windows
export LANG=C

# Build applications
echo Building Cauldron applications
pushd $build
${src}/development/bootstrap.csh \
      -DCMAKE_BUILD_TYPE=${configuration} \
      -DBM_VERSION_NUMBER_MAJOR=${version_number_major} \
      -DBM_VERSION_NUMBER_MINOR=${version_number_minor} \
      -DBM_VERSION_TAG=${version_tag} \
      -DBM_UNIT_TEST_OUTPUT_DIR=${unit_test_output} \
      -DCMAKE_INSTALL_PREFIX=${installdir} \
	  "$@" \
  || { echo error: Configuration has failed; exit 1; } 

source envsetup.sh

# Run Gnu Make :-)
# Notes on parameters:
#   -k  => build as much as you can
#   -j  => parallel build
#   3>&1 1>&2 2>&3 3>&- | sed -e 's/[Ww]arning:/warning :/' 
#       => Redirects stderr to the sed script which makes GCC outputted
#          warnings easier to digest for TFS / Visual Studio
make -k -j${nprocs} 3>&1 1>&2 2>&3 3>&- | sed -e 's/[Ww]arning:/warning :/' \
   || { echo error: Build has failed; exit 1 ; } 


if [ x$deploy = xTrue ]; then
   if [[ ${configuration} =~ "[Rr]elease" ]]; then
      make install/strip || { echo error: Installation has failed; exit 1 ; }
   else
      make install || { echo error: Installation has failed; exit 1 ; } 
   fi
fi

#Give access for the group g_psaz00 to build folder
chgrp -R g_psaz00 $build
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
     bash MoveInstalls.sh || { echo error: Deployment has failed; exit 1 ; }
     echo "Again, trying to install fresh binaries..."
     bash InstallAll.sh || { echo error: Deployment has failed; exit 1 ; }
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
${CTEST} || { echo error: One or more unit tests have failed; exit 1 ; } 

popd

