#!/bin/bash

# This script is meant to set up the environment in Shell for regression tests.
# It will be sourced by *.ini files.

intelCmplModuleName="intel/2019a"

loadedMdl=$(module list 2>&1 | grep ${intelCmplModuleName})
if [ "x${loadedMdl}" == "x" ]; then
   availMdl=$(module av 2>&1 | grep ${intelCmplModuleName})
   if [ "x${availMdl}" == "x" ]; then
      [[ -r /glb/apps/hpc/Lmod/etc/profile.d/z01_lmod-hpcs.sh ]] && . /glb/apps/hpc/Lmod/etc/profile.d/z01_lmod-hpcs.sh 
      module load HpcSoftwareStack/PRODUCTION
   fi
   module purge
   module load ${intelCmplModuleName}
   module load Qt5/5.10.1-intel-2019a
   module load GCCcore/8.2.0
   export LANG="en_US.UTF-8"
fi
loadedMdl=$(module list 2>&1 | grep ${intelCmplModuleName})
if [ "x${loadedMdl}" == "x" ]; then
   echo "Fail to load module ${intelCmplModuleName}. Exiting..."
   exit 1
fi
