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

#### Define the 'extra_cmake_params' variable
set extra_cmake_params

### Determine the installion directory, and save it in 'extra_cmake_params'
set extra_cmake_params = ($extra_cmake_params "-DCMAKE_INSTALL_PREFIX=$cwd")

#### Determine the Configuration (Release or Debug?)
# It can be determined by supplying a command line parameter 
# to this script in the form of '-DCMAKE_BUILD_TYPE=...', 
# by setting the enviornment variable CONFIGURATION, or
# by naming of the build directory in a special way

# So, find the -DCMAKE_BUILD_TYPE param
set cmake_param_defines_config = 0
@ arg = 1
while ($arg <= $#)
   switch($argv[$arg])
   case "-DCMAKE_BUILD_TYPE=*"
      set cmake_param_defines_config = 1
      breaksw
   endsw
   @ arg++
end

# Store the CMake parameter it there was none yet in the positional parameters
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
   set extra_cmake_params = ($extra_cmake_params "-DCMAKE_BUILD_TYPE=$configuration")
endif

#### Copy the command line/positional parameters to '$cmake_params_N' variables, and clear
# the '$argv' variable, because it will confuse the Intel scripts that are sourced.
# The parameters are copied in this funny way, because the parameters could contain white space.
set cmake_params
@ arg = 1
while ( $arg <= $# )
   set cmake_params_${arg} = ( $argv[$arg] )
   set cmake_params = ( $cmake_params '"$cmake_params_'${arg}'"' )
   @ arg++
end
# Clear the positional parameters
set argv

#### Set-up the environment.
setenv MPI_ROOT "/nfs/rvl/apps/3rdparty/intel/impi/4.1.1.036/intel64"
setenv HDF5_ROOT "/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/hdf5-intel-parallel/1.8.11/LinuxRHEL64_x86_64_57WS"

set cauldron_tools = /nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools
set path = ( /bin /usr/bin /usr/local/bin $HOME/bin $HDF5_ROOT/bin )

setenv CMAKE_HOME $cauldron_tools/cmake/cmake-2.8.10.2/Linux64x_26

unsetenv QTDIR
source /etc/profile.d/qt.csh

source /nfs/rvl/apps/3rdparty/intel/ics2013/composer_xe_2013.5.192/bin/compilervars.csh intel64
source $MPI_ROOT/bin/mpivars.csh
source /nfs/rvl/apps/3rdparty/intel/itac/8.1.2.033/bin/itacvars.csh

setenv CXX icpc
setenv CC icc

### Remove old CMake cache
rm -rf CMakeCache.txt CMakeFiles

### Run CMake
# Note: the 'eval' is necessary to evaluate the parameters stored in $cmake_params, i.e. the $cmake_params_N parameters
eval $CMAKE_HOME/bin/cmake $extra_cmake_params $cmake_params $source_directory || exit 1

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
