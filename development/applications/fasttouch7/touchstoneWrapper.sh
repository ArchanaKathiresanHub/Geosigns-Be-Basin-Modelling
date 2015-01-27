#!/bin/bash

#This script is copied directly in the installation directory and not generated by cmake

dir=`dirname $0`

TSLIB_LIBRARY_DIR="${GEOCOSMDIR}"
MCR_ROOT="${MATLABMCR}"
export XAPPLRESDIR="${MCR_ROOT}/X11/app-defaults" 
export LD_LIBRARY_PATH="${MCR_ROOT}/runtime/glnxa64:${MCR_ROOT}/bin/glnxa64:${MCR_ROOT}/sys/os/glnxa64:${MCR_ROOT}/sys/java/jre/glnxa64/jre/lib/amd64/native_threads:${MCR_ROOT}/sys/java/jre/glnxa64/jre/lib/amd64/server:${MCR_ROOT}/sys/java/jre/glnxa64/jre/lib/amd64:${TSLIB_LIBRARY_DIR}:\$LD_LIBRARY_PATH"
export LANG="en_US.UTF-8" 

file="java.opts"
if [ ! -f $file ]
then
	echo "-Xmx4096m" > $file
fi

$dir/touchstoneWrapper "$@" > /dev/null 
