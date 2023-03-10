#!/bin/bash

# This script is meant to set up the environment in Shell for regression tests.
# It will be sourced by *.ini files.

intelCmplModuleName="intel/2019a"

loadedMdl=$(module list 2>&1 | grep ${intelCmplModuleName})
if [ "x${loadedMdl}" == "x" ]; then
   availMdl=$(module av 2>&1 | grep ${intelCmplModuleName})
   if [ "x${availMdl}" == "x" ]; then
      [[ -r /glb/apps/hpc/EasyBuild/Public/Lmod/etc/profile.d/z01_lmod-hpcti.sh ]] && . /glb/apps/hpc/EasyBuild/Public/Lmod/etc/profile.d/z01_lmod-hpcti.sh 
      module load HpcSoftwareStack/PRODUCTION
   fi
	module purge
	module load ${intelCmplModuleName}
	module load Qt5/5.10.1-intel-2019a
	##module load GCCcore/8.2.0
	module load CMake/3.9.6
	#module swap HpctiSoftwareStack/PRODUCTION HpctiSoftwareStack/ACCEPTANCE
	module load HpctiSoftwareStack/PRODUCTION
	module load Doxygen/1.8.15-GCCcore-8.2.0
   export LANG="en_US.UTF-8"
fi
loadedMdl=$(module list 2>&1 | grep ${intelCmplModuleName})
if [ "x${loadedMdl}" == "x" ]; then
   echo "Fail to load module ${intelCmplModuleName}. Exiting..."
   exit 1
fi
