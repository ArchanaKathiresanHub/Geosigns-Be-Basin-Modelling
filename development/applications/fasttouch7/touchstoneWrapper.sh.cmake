#!/bin/bash


export GEOCOSMBASEDIR="${TSLIB_ROOT}" 
export GEOCOSMDIR="${TSLIB_LIBRARY_DIR}"
export MATLABMCR="${MCR_ROOT}" 
export XAPPLRESDIR="${MCR_ROOT}/X11/app-defaults" 
export LD_LIBRARY_PATH="${MCR_ROOT}/runtime/glnxa64:${MCR_ROOT}/bin/glnxa64:${MCR_ROOT}/sys/os/glnxa64:${MCR_ROOT}/sys/java/jre/glnxa64/jre/lib/amd64/native_threads:${MCR_ROOT}/sys/java/jre/glnxa64/jre/lib/amd64/server:${MCR_ROOT}/sys/java/jre/glnxa64/jre/lib/amd64:${TSLIB_LIBRARY_DIR}:\$LD_LIBRARY_PATH"
export LANG="en_US.UTF-8" 
export DISPLAY=

${CMAKE_INSTALL_PREFIX}/applications/fasttouch7/touchstoneWrapper "$@" > /dev/null 2>&1
