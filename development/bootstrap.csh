#!/bin/csh
#
# Setup initial environment in the build directory and run CMAKE
#

# save source path
pushd `dirname $0`
set source_directory = `pwd`
popd

# save build path
set build_directory = $cwd

# parse command line parameters
set cmake_params
set cmake_param_defines_config = 0
foreach arg ($*)
   set cmake_params=($cmake_params $arg)
   shift

   # when one of the parameters defines the configuration, remember that!
   switch($arg)
   case "-DCMAKE_BUILD_TYPE=*"
      set cmake_param_defines_config = 1
      breaksw
   endsw
end

# set environment
setenv MPI_ROOT "/nfs/rvl/apps/3rdparty/intel/ics2011/impi/4.0.1.007/intel64"
setenv HDF5_ROOT "/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/hdf5-intel-parallel/1.8.7/LinuxRHEL64_x86_64_57WS"

set cauldron_tools = /nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools
set path = ( /bin /usr/bin /usr/local/bin $HOME/bin $HDF5_ROOT/bin )

setenv CMAKE_HOME $cauldron_tools/cmake/cmake-2.8.10.2/Linux64x_26

unsetenv QTDIR
source /etc/profile.d/qt.csh

source /apps/3rdparty/intel/ics2011/composerxe-2011.3.174/bin/compilervars.csh intel64
source /apps/3rdparty/intel/ics2011/impi/4.0.1.007/intel64/bin/mpivars.csh
source /apps/3rdparty/intel/ics2011/itac/8.0.1.009/bin/itacvars.csh

setenv CXX icpc
setenv CC icc

# Determine the Configuration (Release or Debug?)
# It can be determined by supplying a command line parameter 
# to this script in the form of '-DCMAKE_BUILD_TYPE=...', 
# by setting the enviornment variable CONFIGURATION, or
# by naming of the build directory in a special way
if ( ! $cmake_param_defines_config ) then
   if ( ${?CONFIGURATION} ) then
      set configuration = $CONFIGURATION
   else
      switch(`basename $build_directory`)
      case "*Debug*"
      case "*debug*"
          set configuration = Debug
          breaksw
      case "*Release*"
      case "*release*"
      default:
          set configuration = Release
      endsw
   endif
    
   echo "CONFIGURATION = $configuration"
   set cmake_params = ( -DCMAKE_BUILD_TYPE=$configuration $cmake_params )
endif

# remove old CMake cache
rm -rf CMakeCache.txt CMakeFiles

$CMAKE_HOME/bin/cmake $source_directory \
    -DCMAKE_INSTALL_PREFIX=`pwd` \
    $cmake_params \
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
