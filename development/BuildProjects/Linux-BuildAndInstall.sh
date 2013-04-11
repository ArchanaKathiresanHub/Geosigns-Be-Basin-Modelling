#!/bin/bash

set -e

# Reading parameters
svnRepository=file:///nfs/rvl/groups/ept-sg/SWEast/Cauldron/SUBVERSION/repository
src=${SRC_DIR:-.}
build=${BUILD_DIR:-/scratch/cauldron_build}
install=${INSTALL_DIR:-/scratch/cauldron_build}
platform=${PLATFORM:-Linux}
configuration=${CONFIGURATION:-Release}
nprocs=${NUMBER_OF_CORES:-8}

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

# Import (legacy) Geocase
${SVN} export -q ${svnRepository}/trunk/development/geocase/BB $src/development/geocase/BB
${SVN} export -q ${svnRepository}/trunk/development/geocase/misc/GospelScriptFiles $src/development/geocase/misc/GospelScriptFiles

pushd $build
csh -x <<EOF
${src}/development/bootstrap.csh || ( echo error: Configuration has failed; exit 1; ) || exit 1
source envsetup.csh 
make -j${nprocs} || ( echo error: Build has failed; exit 1 ; ) || exit 1
${CTEST} || ( echo error: One or more unit tests have failed; exit 1; ) || exit 1
make install/strip || ( echo error: Installation has failed; exit 1 ;) || exit 1
EOF

rm -rf geocase_build
mkdir -p geocase_build
pushd geocase_build
CC=gcc34 CXX=g++34 ${CMAKE} ${src}/development/geocase/BB_Lists \
    -DCMAKE_INSTALL_PREFIX=${build} \
    -DCMAKE_BUILD_TYPE=${configuration}
make -j${nprocs}
make install/strip
popd


echo "Rotate installations"
csh -x ./MoveInstalls.csh || { echo error: Deployment has failed; exit 1 ; }
echo "Install fresh binaries"
csh -x ./InstallAll.csh || { echo error: Deployment has failed; exit 1 ; }
popd
