#!/bin/bash

# setting variables
export PATH=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/TEE-CLC/TEE-CLC-11.0.0:$PATH

WORKSPACE=RegressionTestBench
COLLECTION=https://tfs.sede-coe.pds.nl/tfs/COE-II
ROOTDIRECTORY=$HOME/RegressionTestBench

# Creating and mapping the workspace
if tf workspaces -collection:${COLLECTION} -computer:* | grep -q ^${WORKSPACE} ; then
    echo "- deleting existing workspace $WORKSPACE"
    tf workspace -noprompt -delete -collection:$COLLECTION $WORKSPACE
fi
echo "- Creating new workspace $WORKSPACE"
tf workspace -noprompt -new -collection:$COLLECTION $WORKSPACE -comment:"RegressionTestBench" -location:local
echo "- Mapping workspace $WORKSPACE to folder $ROOTDIRECTORY"
tf workfold  -noprompt -map -collection:$COLLECTION -workspace:$WORKSPACE \$/Basin\ Modeling/IBS/Tests/RegressionTestBench $ROOTDIRECTORY

# creating directory to hold the scripts
echo "- (Re-)creating work folder $ROOTDIRECTORY"
rm -rf $ROOTDIRECTORY
mkdir -p $ROOTDIRECTORY
cd $ROOTDIRECTORY

echo "`pwd` - Retrieving regression test bench ..."
tf get -recursive -force "$ROOTDIRECTORY"

