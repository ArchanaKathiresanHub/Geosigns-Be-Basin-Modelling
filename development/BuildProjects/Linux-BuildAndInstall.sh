#!/bin/bash

set -e

# Reading parameters
svnRepository=file:///nfs/rvl/groups/ept-sg/SWEast/Cauldron/SUBVERSION/repository
src=${SRC_DIR:-.}
build=${BUILD_DIR:-/scratch/cauldron_build}
platform=${PLATFORM:-Linux}
configuration=${CONFIGURATION:-Release}
nprocs=${NUMBER_OF_CORES:-8}
version_number_major=${VERSION_NUMBER_MAJOR:-2013}
version_number_minor=${VERSION_NUMBER_MINOR:-01}
version_tag=${VERSION_TAG:-nightly}

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
CMAKE=/apps/3rdparty/share/cmake
CTEST=/apps/3rdparty/share/ctest

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

csh -x <<EOF
source envsetup.csh 
make -j${nprocs} || ( echo error: Build has failed; exit 1 ; ) || exit 1
${CTEST} || ( echo error: One or more unit tests have failed; exit 1; ) || exit 1

if ( ${configuration} =~ [Dd]ebug ) then
   make install || ( echo error: Installation has failed; exit 1 ;) || exit 1
else
   make install/strip || ( echo error: Installation has failed; exit 1 ;) || exit 1
endif
EOF


# Import (legacy) Geocase
echo Building Cauldron Geocase applications
rm -rf $src/development/geocase/BB $src/development/geocase/misc/GospelScriptFiles
${SVN} export -q ${svnRepository}/trunk/development/geocase/BB $src/development/geocase/BB
${SVN} export -q ${svnRepository}/trunk/development/geocase/misc/GospelScriptFiles $src/development/geocase/misc/GospelScriptFiles

# Build Geocase applications
rm -rf geocase_build
mkdir -p geocase_build
pushd geocase_build
CC=gcc34 CXX=g++34 ${CMAKE} ${src}/development/geocase/BB_Lists \
    -DCMAKE_INSTALL_PREFIX=${build} \
    -DCMAKE_BUILD_TYPE=${configuration}
make -j${nprocs}
make install/strip
popd

# Install
echo "Rotate installations"
csh -x ./MoveInstalls.csh || { echo error: Deployment has failed; exit 1 ; }
echo "Install fresh binaries"
csh -x ./InstallAll.csh || { echo error: Deployment has failed; exit 1 ; }
popd
