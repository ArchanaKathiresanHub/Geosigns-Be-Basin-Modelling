#!/bin/bash

#
# Setup initial environment in the build directory and run CMAKE
#

##########################################################################
### Define default modules name for Intel C++ and Intel MPI
intel_cxx_module_name="intel/2016.01"
intel_impi_module_name="impi/5.1.2.150-iccifort-2016.1.150-GCC-4.9.3-2.25"
intel_imkl_module_name="imkl/11.3.1.150-iimpi-2016.01-GCC-4.9.3-2.25"

##########################################################################

##########################################################################
# Nothing should be changed after this line
##########################################################################
# save source path
pushd `dirname $0` > /dev/null
source_directory=`pwd`
popd > /dev/null

# save build path
build_directory=$PWD

#### Define the 'extra_cmake_params' variable

### Determine the installion directory, and save it in 'extra_cmake_params'
extra_cmake_params+=" -DCMAKE_INSTALL_PREFIX=${build_directory}"

# Scan arguments for some settings
cmake_param_defines_config=0
module_loaded_in_script=0

#Intel C++ compiler
cmake_param_use_intel=1
cmake_param_intel_root=0
intel_path=""
intel_version=""

#Intel MPI
cmake_param_use_impi=1
cmake_param_impi_root=0
impi_path=""
impi_version=""

#Intel MKL
cmake_param_use_imkl=1
cmake_param_imkl_root=0
imkl_path=""
imkl_version=""


for arg in "$@"
do
   case $arg in
      *CMAKE_BUILD_TYPE=*)           # build type is givien as command line parameter
         cmake_param_defines_config=1
         ;;

      *USE_INTEL_COMPILER=OFF)       # Intel compiler is switched OFF in command line
         cmake_param_use_intel=0
         ;;

      *INTEL_CXX_ROOT*)             # path to Intel compiler is givien as command line parameter
         cmake_param_intel_root=1
         ;;

      *INTEL_MPI_ROOT*)             # path to Intel MPI is given as command line parameter
         cmake_param_impi_root=1
         ;;

      *INTEL_MKL_ROOT*)             # path to Intel MKL is given as command line parameter
         cmake_param_imkl_root=1
         ;;
   esac
done


##################################################
# Load modules environment:
[[ -r /glb/data/hpcrnd/easybuild/public/etc/profile.d/shell-envmodules.sh ]] && . /glb/data/hpcrnd/easybuild/public/etc/profile.d/shell-envmodules.sh

##################################################
# Check/load module with settings for Intel C++ compiler
if [ $cmake_param_use_intel -eq 1 ] && [ $cmake_param_intel_root -eq 0 ] ; then

   # check first that intel c++ module is already loaded
   intel2016av=`module list 2>&1 | grep ${intel_cxx_module_name}`

   if [ "x${intel2016av}" == "x" ]; then

      intel2016av=`module av ${intel_cxx_module_name} 2>&1`
      if [ "x${intel2016av}" == "x" ]; then
         echo "Can't find module ${intel_cxx_module_name} to load environment settings for Intel C++ compiler"
         exit 1;
      else
         echo "Loading module for Intel C++ ..."
         module load ${intel_cxx_module_name}
         module_loaded_in_script=1
      fi
   fi
   # extract path to intel compiler installation and it version
   icc_path=`which icc`
   intel_path=`dirname ${icc_path}`   #remove icc from path
   intel_path=`dirname ${intel_path}` #remove intel64 from path
   intel_path=`dirname ${intel_path}` #remove bin from path

   # check that we got path to the intel compiler installataion
   if [ "x${intel_path}" == "x" ]; then
      echo "Can't load module for Intel C++ 2016.01"
      exit 1;
   fi
fi

# Add to cmake parameter the path and version number for intel compiler
if [ "x${intel_path}" != "x" ]; then
   extra_cmake_params+=" -DINTEL_CXX_ROOT=${intel_path}"
fi

if [ "x${intel_version}" != "x" ]; then
   extra_cmake_params+=" -DINTEL_CXX_VERSION=\"${intel_version}\""
fi

#################################################
# Load settings for Intel MPI
if [ $cmake_param_use_impi -eq 1 ] && [ $cmake_param_impi_root -eq 0 ] ; then

   # check first that impi module is already loaded with intel compiler
   impiav=`module list 2>&1 | grep ${intel_impi_module_name}`

   # if not - try to load module
   if [ "x${impiav}" == "x" ]; then
      impiav=`module av 2>&1 | grep ${intel_impi_module_name}`

      if [ "x${impiav}" == "x" ]; then
         echo "Can't find module to load environment settings for Intel MPI"
         exit 1;
      else
         echo "Loading module for Intel MPI ..."
         module load ${intel_impi_module_name}
         module_loaded_in_script=1
      fi
   else
      echo "Module for Intel MPI is already loaded"
   fi

   # extract mpi root and version number
   impi_path=`   module show ${intel_impi_module_name} 2>&1 | grep I_MPI_ROOT    | cut -d ' ' -f 3`
   impi_version=`module show ${intel_impi_module_name} 2>&1 | grep EBVERSIONIMPI | cut -d ' ' -f 3`
fi

# Add to cmake parameters path and version number for Intel MPI
if [ "x${impi_path}" != "x" ]; then
   extra_cmake_params+=" -DINTEL_MPI_ROOT=${impi_path}"
fi

if [ "x${impi_version}" != "x" ]; then
   extra_cmake_params+=" -DINTEL_MPI_VERSION=\"${impi_version}\""
fi



#################################################
# Load settings for Intel MKL
if [ $cmake_param_use_imkl -eq 1 ] && [ $cmake_param_imkl_root -eq 0 ] ; then

   # check first that impi module is already loaded with intel compiler
   imklav=`module list 2>&1 | grep ${intel_imkl_module_name}`

   # if not - try to load module
   if [ "x${imklav}" == "x" ]; then
      imklav=`module av 2>&1 | grep ${intel_imkl_module_name}`

      if [ "x${imklav}" == "x" ]; then
         echo "Can't find module to load environment settings for Intel MKL"
         exit 1;
      else
         echo "Loading module for Intel MKL ..."
         module load ${intel_imkl_module_name}
         module_loaded_in_script=1
      fi
   else
      echo "Module for Intel MKL is already loaded"
   fi

   # extract mpi root and version number
   imkl_path=`   module show ${intel_imkl_module_name} 2>&1 | grep MKLROOT       | cut -d ' ' -f 3`
   imkl_version=`module show ${intel_imkl_module_name} 2>&1 | grep EBVERSIONIMKL | cut -d ' ' -f 3`
fi

# Add to cmake parameters path and version number for Intel MPI
if [ "x${imkl_path}" != "x" ]; then
   extra_cmake_params+=" -DINTEL_MKL_ROOT=${imkl_path}"
fi

if [ "x${imkl_version}" != "x" ]; then
   extra_cmake_params+=" -DINTEL_MKL_VERSION=\"${imkl_version}\""
fi


# Store the CMake parameter it there was none yet in the positional parameters
if [ $cmake_param_defines_config -eq 0 ]; then
   if [ -z "${CONFIGURATION}" ]; then
      case `basename $build_directory` in
      *[Dd]ebug*)
         configuration=Debug
         ;;

      *[Rr]elease*)
         configuration=Release
         ;;

      *)
        configuration=Release
        ;;

      esac
   else
      configuration=${CONFIGURATION}
   fi
   echo "CONFIGURATION = $configuration"
   extra_cmake_params+=" -DCMAKE_BUILD_TYPE=${configuration}"
fi

### Remove old CMake cache
rm -rf CMakeCache.txt CMakeFiles

### Run CMake
if [ -d /nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools ]; then
   cauldron_tools=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools
   cmake=$cauldron_tools/bin/cmake
else
   cmake=`which cmake`
fi

echo "cmake $extra_cmake_params $@"
$cmake $extra_cmake_params $@ $source_directory || exit 1

if [ $module_loaded_in_script -eq 1 ]; then
   echo ""
   echo "Some modules has been loaded in bootstrap script. To compile cauldron you'll need these modules:"
   echo "  ${intel_cxx_module_name}"
   echo "  ${intel_impi_module_name}"
   echo "  ${intel_imkl_module_name}"
   echo "to be loaded before make command. To do so you need to add to your .kshrc/.bashrc file the following lines:"
   echo ""
   echo "  [[ -r /glb/data/hpcrnd/easybuild/public/etc/profile.d/shell-envmodules.sh ]] && . /glb/data/hpcrnd/easybuild/public/etc/profile.d/shell-envmodules.sh"
   echo "  module load ${intel_cxx_module_name}"
   echo ""
   echo "and restart your shell"
fi

#
# Typical usage scenario:
#
#  <get sources>
#  $ mkdir linux_build_debug
#  $ cd linux_build_debug
#  $ ../bootstrap.sh
#  $ source envsetup.sh
#  $ make
#
