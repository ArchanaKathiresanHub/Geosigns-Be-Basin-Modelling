macro( install_casawizard_application TARGETNAME )
if(UNIX)
  install( TARGETS ${TARGETNAME} RUNTIME DESTINATION bin )

  set( SCRIPTNAME "${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}" )
  file( WRITE ${SCRIPTNAME}
"#!/bin/bash

DIR=\"$( cd \"$( dirname \"\${BASH_SOURCE\[0\]}\" )\" && pwd )\"

[[ -r /glb/apps/hpc/EasyBuild/Public/Lmod/etc/profile.d/z01_lmod-hpcti.sh ]] && . /glb/apps/hpc/EasyBuild/Public/Lmod/etc/profile.d/z01_lmod-hpcti.sh
module load HpctiSoftwareStack/PRODUCTION
module purge &> /dev/null

module load imkl/2021.2.0-iimpi-2021a
module load intel/2021a
module load Qt5/5.15.2-GCCcore-10.3.0

EXEC_NAME=${TARGETNAME}.exe
export PATH=$DIR:$PATH
MISCDIR=\"\"
if [ -d \"$DIR/../../misc\" ]; then
  MISCDIR=$DIR/../../misc
else
  if [ -d \"$DIR/../misc\" ]; then
    MISCDIR=$DIR/../misc
  fi
fi
export CTCDIR=$MISCDIR
export GENEXDIR=$MISCDIR/genex40
export GENEX5DIR=$MISCDIR/genex50
export GENEX6DIR=$MISCDIR/genex60
export OTGCDIR=$MISCDIR/OTGC
export EOSPACKDIR=$MISCDIR/eospack

$DIR/$EXEC_NAME
" )
  install( PROGRAMS ${SCRIPTNAME} DESTINATION bin )
endif(UNIX)
endmacro( install_casawizard_application )
