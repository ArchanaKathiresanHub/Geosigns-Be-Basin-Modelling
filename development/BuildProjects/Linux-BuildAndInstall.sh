#!/bin/bash

# Reading parameters with some defaults
svnRepository=file:///nfs/rvl/groups/ept-sg/SWEast/Cauldron/SUBVERSION/repository
src=${SRC_DIR:-`dirname $0`/../..}
build=${BUILD_DIR:-`mktemp -d`}
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

# Build applications
echo Building Cauldron applications
pushd $build
${src}/development/bootstrap.csh \
      -DCMAKE_BUILD_TYPE=${configuration} \
      -DBM_VERSION_NUMBER_MAJOR=${version_number_major} \
      -DBM_VERSION_NUMBER_MINOR=${version_number_minor} \
      -DBM_VERSION_TAG=${version_tag} \
	  "$@" \
  || { echo error: Configuration has failed; exit 1; } 

source envsetup.sh

make -k -j${nprocs} || { echo error: Build has failed; exit 1 ; } 

#Give access for the group g_psaz00 to build folder
chgrp -R $build
chmod -R g+rw $build

if [[ ${configuration} =~ "[Dd]ebug" ]]; then
   make install || { echo error: Installation has failed; exit 1 ; }
else
   make install/strip || { echo error: Installation has failed; exit 1 ; } 
fi

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
       -DCMAKE_INSTALL_PREFIX=${build} \
       -DCMAKE_BUILD_TYPE=${configuration}
   make -j${nprocs}
   make install/strip
   popd
else
   echo Geocase applications have been excluded
fi

# Install to SSSDEV
if [ x$deploy = xTrue ]; then
   echo "Rotate installations"
   bash MoveInstalls.sh || { echo error: Deployment has failed; exit 1 ; }
   echo "Install fresh binaries"
   bash InstallAll.sh || { echo error: Deployment has failed; exit 1 ; }

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

