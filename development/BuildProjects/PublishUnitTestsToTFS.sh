#!/bin/bash

# SHOULD BE ALREADY DEFINED
#TFS_SERVERURL=https://tfs.sede-coe.pds.nl/tfs/COE-II
#JOB_NAME=Linux_Nightly_BuildAndInstall
#TFS_PROJECTPATH='$/Basin Modeling/IBS/Trunk'
#JENKINS_HOME=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/jenkins
#BUILD_NUMBER=51
#

WORKSPACE=${JOB_NAME}_results
#Replace none ntfs characters for Jenkins multiconfiguration projects (replace = and / by _)
WORKSPACE=`echo $WORKSPACE | sed "s/=/_/g"`
WORKSPACE=`echo $WORKSPACE | sed "s/\//_/g"`
UTF=UnitTestResults

TF=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/bin/tf
pushd build


mkdir -p ${UTF}
pushd ${UTF}

echo "${TF} workspaces -collection:${TFS_SERVERURL} | grep ${WORKSPACE}"
CHKWS=`${TF} workspaces -collection:${TFS_SERVERURL} | grep ${WORKSPACE} | cut -d ' ' -f 1`
echo $CHKWS

if [ "x${CHKWS}" != "x" ]; then
    echo Deleting workspace: $WORKSPACE
    $TF workspace -delete ${WORKSPACE} -collection:${TFS_SERVERURL}
fi

echo "Creating workspace: ${WORKSPACE}"
$TF workspace -new ${WORKSPACE} -collection:${TFS_SERVERURL} -location:local

echo "Mapping folder"
echo "$TF workfold -map -workspace:${WORKSPACE} -collection:${TFS_SERVERURL} ${TFS_PROJECTPATH}/../Builds/Jenknis/${JOB_NAME} `pwd`"

$TF workfold -map -workspace:${WORKSPACE} -collection:${TFS_SERVERURL} "${TFS_PROJECTPATH}/../Builds/Jenkins/${JOB_NAME}" `pwd`
$TF get -force .

# check, update and delete if xml files from TFS do not exist in the build
FORDELETE=""
for f in `find . -maxdepth 1 -name '*.xml'`
do
    if [ -f ../${f} ]; then
       echo "Copy: $f"
       rm -f ./$f
       cp ../$f .
    else
       echo "Mark to delete $f"
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

#check was the build succeded and genertate script
if [ ! -f check_build.bat ]; then
  touch check_build.bat
  FORADDING="${FORADDING} check_build.bat"
fi

#check was unit tests are succeded and genertate script
if [ ! -f check_utests.bat ]; then
  touch check_utests.bat
  FORADDING="${FORADDING} check_utests.bat"
fi

#copy job log
if [ ! -f log ]; then
  FORADDING="${FORADDING} log"
fi
cp ${JENKINS_HOME}/jobs/${JOB_NAME}/builds/${BUILD_NUMBER}/log .

# Generate script which will print log and return 0 on success build or 1 on failure
echo "type log" > check_build.bat
echo "REM Build: ${JOB_NAME}_${BUILD_TIMESTAMP}_${BUILD_NUMBER} was" >> check_build.bat
if [ -f ../BuildSucceeded ]; then
  echo "REM Succeeded! :0)" >> check_build.bat
  echo "exit 0" >> check_build.bat
else
  echo "REM Failed :0(" >> check_build.bat
  echo "exit 1" >> check_build.bat
fi

# Generate script which will return 0 on success of unit tests or 1 on failure
echo "REM Unit tests for the build: ${JOB_NAME}_${BUILD_TIMESTAMP}_${BUILD_NUMBER} were" > check_utests.bat
if [ -f ../UnitTestsSucceeded ]; then
  echo "REM Succeeded! :0)" >> check_utests.bat
  echo "exit 0" >> check_utests.bat
else
  echo "REM Failed :0(" >> check_utests.bat
  echo "exit 1" >> check_utests.bat
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

echo "Check in changes to TFS"
$TF ci -recursive .

echo "Delete workspace as not needed any more ${WORKSPACE}"
$TF workspace -delete ${WORKSPACE} -collection:${TFS_SERVERURL}

popd
rm -rf ${UTF}
popd
