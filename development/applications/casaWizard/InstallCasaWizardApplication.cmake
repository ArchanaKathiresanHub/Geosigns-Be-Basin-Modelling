macro( install_casawizard_application TARGETNAME )
if(UNIX)
  install( TARGETS ${TARGETNAME} RUNTIME DESTINATION bin )

  set( SCRIPTNAME "${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}" )
  file( WRITE ${SCRIPTNAME}
"#!/bin/bash

DIR=$(dirname $0)
. /glb/apps/hpc/Lmod/etc/profile.d/z01_lmod-hpcs.sh
module load HpcSoftwareStack/PRODUCTION
module purge &> /dev/null
module load Qt5/5.10.1-intel-2019a

EXEC_NAME=${TARGETNAME}.exe
export PATH=$DIR:$PATH
if [ -d \"$DIR/../../misc\" ]; then
  export CTCDIR=$DIR/../../misc
else
  if [ -d \"$DIR/../misc\" ]; then
    export CTCDIR=$DIR/../misc
  fi
fi
$(dirname $0)/$EXEC_NAME
" )
  install( PROGRAMS ${SCRIPTNAME} DESTINATION bin )
endif(UNIX)
endmacro( install_casawizard_application )
