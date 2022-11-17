macro( install_casawizard_application TARGETNAME )
if(UNIX)
  install( TARGETS ${TARGETNAME} RUNTIME DESTINATION bin )

  set( SCRIPTNAME "${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}" )
  file( WRITE ${SCRIPTNAME}
"#!/bin/bash

DIR=\"$( cd \"$( dirname \"\${BASH_SOURCE\[0\]}\" )\" && pwd )\"

[[ -r /glb/apps/hpc/EasyBuild/Public/Lmod/etc/profile.d/z01_lmod-hpcti.sh ]] && . /glb/apps/hpc/EasyBuild/Public/Lmod/etc/profile.d/z01_lmod-hpcti.sh
module load HpcSoftwareStack/PRODUCTION
module purge &> /dev/null
module load Qt5/5.10.1-intel-2019a

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
