#!/bin/bash

# Reading parameters with some defaults
src=${SRC_DIR:-`dirname $0`/../..}
build=${BUILD_DIR:-`mktemp -d`}
platform=${PLATFORM:-Linux}
configuration=${CONFIGURATION:-Release}
nprocs=${NUMBER_OF_CORES:-20}

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
CMAKE=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/cmake/cmake-2.8.10.2/Linux64x_26/bin/cmake
CTEST=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/cmake/cmake-2.8.10.2/Linux64x_26/bin/ctest

# Enable editing of the files
EXTERNAL_COMPONENTS_DIR=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc-library
chmod -R u+w ${EXTERNAL_COMPONENTS_DIR}

# Build the libraries for static and dynamic link modes
for buildSharedLibs in OFF ON
do
   # Build applications
   echo "Building Cauldron applications (SHARED_LIBS = $buildSharedLibs)"
   pushd $build
   rm -Rf *

   # configure
   ${src}/development/bootstrap.csh \
      -DCMAKE_BUILD_TYPE=${configuration} "$@" \
      -DBUILD_SHARED_LIBS=${buildSharedLibs} \
      -DBM_EXTERNAL_COMPONENTS_REBUILD=ON \
      -DBM_EXTERNAL_COMPONENTS_DIR=${EXTERNAL_COMPONENTS_DIR} \
     || { echo error: Configuration has failed; exit 1; } 

   # Source environment (otherwise the external libraries don't build)    
   source envsetup.sh

   # Build everything
   make -k -j${nprocs} || { echo error: Build has failed; exit 1 ; } 

   # Run Unit Tests
   ${CTEST} || { echo error: One or more unit tests have failed; exit 1 ; } 

   popd
done

# Update the permissions of the library cache: Make it read-only
chmod -R u-w,g-w,o-w,a+r ${EXTERNAL_COMPONENTS_DIR}
