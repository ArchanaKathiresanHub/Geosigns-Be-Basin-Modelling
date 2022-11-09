#!/bin/bash

# This script is meant to set up the environment in Shell for regression tests.
# It will be sourced by *.ini files.

intelCmplModuleName="intel/2021a"

loadedMdl=$(module list 2>&1 | grep ${intelCmplModuleName})
if [ "x${loadedMdl}" == "x" ]; then
   availMdl=$(module av 2>&1 | grep ${intelCmplModuleName})
   if [ "x${availMdl}" == "x" ]; then
      [[ -r /glb/apps/hpc/EasyBuild/Public/Lmod/etc/profile.d/z01_lmod-hpcti.sh ]] && . /glb/apps/hpc/EasyBuild/Public/Lmod/etc/profile.d/z01_lmod-hpcti.sh 
      module load HpctiSoftwareStack/PRODUCTION
   fi
	module purge
	module load ${intelCmplModuleName}
	module load CMake/3.18.4
	module load imkl/2021.2.0-iimpi-2021a
	module load Doxygen/1.9.1-GCCcore-10.3.0 Qt5/5.15.2-GCCcore-10.3.0
   export LANG="en_US.UTF-8"
fi
loadedMdl=$(module list 2>&1 | grep ${intelCmplModuleName})
if [ "x${loadedMdl}" == "x" ]; then
   echo "Fail to load module ${intelCmplModuleName}. Exiting..."
   exit 1
fi
