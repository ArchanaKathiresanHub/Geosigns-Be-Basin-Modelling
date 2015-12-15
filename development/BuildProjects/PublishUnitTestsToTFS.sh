#!/bin/bash

# SHOULD BE ALREADY DEFINED
#TFS_SERVERURL=https://tfs.sede-coe.pds.nl/tfs/COE-II
#JOB_NAME=Linux_Nightly_BuildAndInstall
#TFS_PROJECTPATH='$/Basin Modeling/IBS/Trunk'
#

#
WORKSPACE=${JOB_NAME}_results
UTF=UnitTestResults

TF=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/bin/tf
pushd build


mkdir -p ${UTF}
pushd ${UTF}

#echo "${TF} workspaces -collection:${TFS_SERVERURL} | grep ${WORKSPACE}"
CHKWS=`${TF} workspaces -collection:${TFS_SERVERURL} | grep ${WORKSPACE} | cut -d ' ' -f 1`
#echo $CHKWS

if [ "x${CHKWS}" != "x" ]; then
#    echo Deleting workspace: $WORKSPACE
    $TF workspace -delete ${WORKSPACE} -collection:${TFS_SERVERURL}
fi

#echo "Creating workspace: ${WORKSPACE}"
$TF workspace -new ${WORKSPACE} -collection:${TFS_SERVERURL} -location:local

#echo "Mapping folder"
#echo "$TF workfold -map -workspace:${WORKSPACE} -collection:${TFS_SERVERURL} ${TFS_PROJECTPATH}/../Builds/Jenknis/ `pwd`"

$TF workfold -map -workspace:${WORKSPACE} -collection:${TFS_SERVERURL} "${TFS_PROJECTPATH}/../Builds/Jenkins/${JOB_NAME}/" `pwd`
$TF get -force .

# check, update and delete if xml files from TFS do not exist in the build
FORDELETE=""
for f in `find . -maxdepth 1 -name '*.xml'`
do
    if [ -f ../${f} ]; then
#       echo "Copy: $f"
       rm -f ./$f
       cp ../$f .
    else
#       echo "Mark to delete $f"
       FORDELETE="${FORDELETE} ${f}"
    fi
done

# check and add if they are not exist it TFS xml files from the build
FORADDING=""
for f in `find .. -maxdepth 1 -name '*.xml'`
do
   ff=`basename $f`

   if [ ! -f $ff ]; then
      cp $f $ff
      FORADDING="${FORADDING} ${ff}"
   fi
done

#check was build succeded and genertate script
if [ ! -f check_build.bat ]; then
  touch check_build.bat
  FORADDING="${FORADDING} check_build.bat"
fi

#copy job log
if [ ! -f log ]; then
  FORADDING="${FORADDING} log"
fi
cp ${JENKINS_HOME}/jobs/${JOB_NAME}/builds/${BUILD_NUMBER}/log .

# Generate script which will print log and return 0 on success build or 1 on failure
echo "type log" >> check_build.bat
if [ -f ../BuildSucceeded ]; then
  echo "exit 0" >> check_build.bat
else
  echo "exit 1" >> check_build.bat
fi

#echo "Will be deleted: $FORDELTE"
if [ "x${FORDELETE}" != "x" ]; then
#   echo "Delete from TFS ${FORDELETE}"
   ${TF} delete ${FORDELETE}
fi

#echo "Will be added: $FORADDING"
if [ "x${FORADDING}" != "x" ]; then
#   echo "Add to TFS ${FORADDING}"
   ${TF} add ${FORADDING}
fi

$TF ci -recursive .

# delete workspace as not needed any more
$TF workspace -delete ${WORKSPACE} -collection:${TFS_SERVERURL}

popd
rm -rf ${UTF}
popd
