#!/bin/bash

PROJECTFILE=$1

if [ x$PROJECTFILE = x  ]; then
  echo Usage: $0 PROJECTFILE
  echo where PROJECTFILE should be substituted with the project3d file name.
  echo This runs the fastcauldron simulator in hydrostatic mode.
  exit 1
fi

echo SETTING UP ENVIRONMENT
. /nfs/rvl/groups/ept-sg/SWEast/Cauldron/inversion/setenv.sh
echo
echo RUNNING FASTCAULDRON
/nfs/rvl/groups/ept-sg/SWEast/Cauldron/inversion/src/c++/cauldron/applications/fastcauldron/obj.Linux64/fastcauldron -project $PROJECTFILE -temperature || exit 2
echo
echo RUNNING DATADRILLER
/nfs/rvl/groups/ept-sg/SWEast/Cauldron/inversion/src/c++/cauldron/applications/datadriller/obj.Linux64/datadriller -input $PROJECTFILE -output $PROJECTFILE || exit 3
echo DONE
