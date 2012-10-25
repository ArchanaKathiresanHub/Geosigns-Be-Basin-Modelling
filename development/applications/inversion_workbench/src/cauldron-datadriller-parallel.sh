#!/bin/bash

EXTRA_CAULDRON_PARAMS=-temperature
CAULDRON_SRC_DIR=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/inversion/src/c++/cauldron

# checking parameters: All Project3D files should be in the same directory
echo CHECKING PARAMATERS
if [ x$1 = x ]; then
  echo Warning: No input files given
  echo DONE
  exit 1
fi

dir="$( dirname $1 )"

for arg
do
  if [ "$dir" != "$(dirname $arg)" ]; then
    echo Error: All project3d files must be in the same directory
    exit 1
  fi
done


echo 
echo SETTING UP ENVIRONMENT
. /nfs/rvl/groups/ept-sg/SWEast/Cauldron/inversion/setenv.sh

NPROCS=$( grep processor /proc/cpuinfo | wc -l ) 
echo Detected $NPROCS processors
makefile=$( mktemp -p $dir Makefile.tmpXXXXXX ) || { echo Could not create temporary Makefile in directory $dir; exit 1; }
echo
echo WRITING MAKE FILE
cat > $makefile <<EOF
%.project3d : PHONY
	$CAULDRON_SRC_DIR/applications/fastcauldron/obj.Linux64/fastcauldron -project \$@ $EXTRA_CAULDRON_PARAMS > \$*_logfile 2>&1
	$CAULDRON_SRC_DIR/applications/datadriller/obj.Linux64/datadriller -input \$@ -output \$@ >> \$*_logfile 2>&1 

PHONY:

EOF

echo EXECUTING MAKE FILE
make -C $dir -f $makefile -j $NPROCS "$@" || { echo PARALLEL EXECUTION FAILED; exit 1; }

echo REMOVING MAKEFILE
rm -Rf $makefile

echo DONE
