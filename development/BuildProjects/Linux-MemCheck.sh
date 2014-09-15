#!/bin/bash

src=${SRC_DIR:-`dirname $0`/../..}
build=${BUILD_DIR:-`mktemp -d`}
installdir=${INSTALL_DIR:-"${build}"}
unit_test_output=${UNIT_TEST_OUTPUT_DIR:-"${build}"}
platform=${PLATFORM:-Linux}
configuration=${CONFIGURATION:-MemCheck}
nprocs=${NUMBER_OF_CORES:-12}

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
VALGRIND_ROOT=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/Valgrind/valgrind-3.9.0-impi-4.1.1.036/Linux
VALGRIND=${VALGRIND_ROOT}/bin/valgrind
VALGRIND_MPI=${VALGRIND_ROOT}/lib/valgrind/libmpiwrap-amd64-linux.so
INTEL_COMPILER_VARS=/nfs/rvl/apps/3rdparty/intel/ics2013/composer_xe_2013.5.192/bin/compilervars.sh

### Build applications
echo Building Cauldron
pushd $build

${CMAKE} ${src}/development \
      -DCMAKE_BUILD_TYPE=${configuration} \
      -DBUILD_SHARED_LIBS=ON  \
      -DBM_USE_INTEL_COMPILER=OFF \
      -DBM_UNIT_TEST_OUTPUT_DIR=${unit_test_output} \
      -DCMAKE_INSTALL_PREFIX=${installdir} \
	  "$@" \
  || { echo error: Configuration has failed; exit 1; } 

source envsetup.sh

make -k -j${nprocs} || { echo error: Build has failed; exit 1 ; } 

### Run Unit Tests
${CTEST} || { echo error: One or more unit tests have failed; exit 1 ; } 


### Run Valgrind memcheck
# Copy the test project to the build dir
cp ${src}/development/BuildProjects/MemCheck.project3d .
# Load the intel compiler variables in the environment, because Valgrind was built with the Intel compiler.
source ${INTEL_COMPILER_VARS} intel64
# Preload the Valgrind MPI wrapper to that MPI won't trigger any errors
export LD_PRELOAD=$VALGRIND_MPI 
# Run valgrind on fastcauldron with the test project. 
# Note: --error-exitcode=1 makes valgrind exit with return code 1 when there is
#                          a memory error 
#       --run-libc-freeres=no is necessary, because glibc versions
#                          until 2.12.90-14 have a bug that triggers an error in Valgrind
#      The output redirection at the end is necessary because TFS would interpret the "ERROR SUMMARY" as an error! 
$VALGRIND --error-exitcode=1 --run-libc-freeres=no --tool=memcheck applications/fastcauldron/fastcauldron -project MemCheck.project3d -itcoupled 2>&1 | sed -e 's/ERROR //' 

