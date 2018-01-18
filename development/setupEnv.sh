#!/bin/ksh

# This script is meant to set up the environment in Shell for regression tests.
# It will be sourced by *.ini files.

intelCmplModuleName="intel/2017.05"

export LD_PRELOAD="/glb/data/hpcrnd/easybuild/PRODUCTION/software/rhel/6/GCCcore/4.9.3/lib64/libstdc++.so"
loadedMdl=$(module list 2>&1 | grep ${intelCmplModuleName})
if [ "x${loadedMdl}" == "x" ]; then
   availMdl=$(module av 2>&1 | grep ${intelCmplModuleName})
   if [ "x${availMdl}" == "x" ]; then
      [[ -r /glb/data/hpcrnd/easybuild/public/etc/profile.d/shell-envmodules.sh ]] && . /glb/data/hpcrnd/easybuild/public/etc/profile.d/shell-envmodules.sh
   fi
   module purge
   module load ${intelCmplModuleName}
fi
loadedMdl=$(module list 2>&1 | grep ${intelCmplModuleName})
if [ "x${loadedMdl}" == "x" ]; then
   echo "Fail to load module ${intelCmplModuleName}. Exiting..."
   exit 1
fi