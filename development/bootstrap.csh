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


### Remove old CMake cache
rm -rf CMakeCache.txt CMakeFiles

### Run CMake
if ( -d /nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools ) then
   set cauldron_tools = /nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools
   set cmake = $cauldron_tools/bin/cmake
else
   set cmake = `which cmake`
endif
$cmake $extra_cmake_params $argv $source_directory || exit 1

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
