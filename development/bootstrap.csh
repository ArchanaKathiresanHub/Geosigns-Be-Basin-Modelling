#!/bin/csh
#
# Setup initial environment in the build directory and run CMAKE
#

pushd `dirname $0`
set source_directory = `pwd`
popd

foreach c ($*)
   shift
end

setenv PATH /bin:/usr/bin:/usr/local/bin:$HOME/bin

setenv MPI_ROOT "/nfs/rvl/apps/3rdparty/intel/ics2011/impi/4.0.1.007/intel64"
setenv HDF5_ROOT "/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/hdf5-intel-parallel/1.8.7/LinuxRHEL64_x86_64_57WS"

setenv PATH $HDF5_ROOT/bin:$PATH

setenv CMAKE_HOME /glb/home/nlyku5/R/cmake-2.8.9-Linux-i386

unsetenv QTDIR
source /etc/profile.d/qt.csh

source /apps/3rdparty/intel/ics2011/composerxe-2011.3.174/bin/compilervars.csh intel64
source /apps/3rdparty/intel/ics2011/impi/4.0.1.007/intel64/bin/mpivars.csh
source /apps/3rdparty/intel/ics2011/itac/8.0.1.009/bin/itacvars.csh

setenv CXX icpc
setenv CC icc

# figure out the Configuration which is either Release or Debug
switch(`basename $cwd`)
case "*Debug*"
case "*debug*"
    set configuration=Debug
    breaksw
case "*Release*"
case "*release*"
default:
    set configuration=Release
endsw

echo "CONFIGURATION = $configuration"

# remove old CMake cache
rm -rf CMakeCache.txt CMakeFiles

$CMAKE_HOME/bin/cmake $source_directory \
    -DCMAKE_BUILD_TYPE=$configuration \
    -DCMAKE_INSTALL_PREFIX=`pwd` \
|| exit 1

#
# Typical usage scenario:
#
#  <get sources>
#  $ mkdir linux_build_debug
#  $ cd linux_build_debug
#  $ ../bootstrap.csh
#  $ source envsetup.csh
#  $ make
#
