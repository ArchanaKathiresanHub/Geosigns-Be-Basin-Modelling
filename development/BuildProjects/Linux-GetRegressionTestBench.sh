#!/bin/bash

######################################################
## Set variables                                   ###
######################################################

### Set the PATH environment variable
tfsTools=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/TEE-CLC/TEE-CLC-11.0.0
export PATH=$tfsTools:/usr/bin/X11:/usr/kerberos/bin:/usr/local/bin:/bin:/usr/bin 

### Set configurable environment variables
if [ "x$SCRIPT_WORKSPACE" = x ]; then
   SCRIPT_WORKSPACE="RegressionTestBench_${RANDOM}"
fi

if [ "x$COLLECTION" = x ]; then
   COLLECTION=https://tfs.sede-coe.pds.nl/tfs/COE-II
fi

if [ "x$SCRIPT_DIRECTORY" = x ]; then
   SCRIPT_DIRECTORY=$HOME/RegressionTestBench
fi

if [ "x$SCRIPT_TFSFOLDER" = x ]; then
   SCRIPT_TFSFOLDER='$/Basin Modeling/IBS/Tests/RegressionTestBench'
fi

######################################################
## Set-up script directory                         ###
######################################################

# Creating and mapping the workspace
if tf workspaces -collection:"${COLLECTION}" | grep -q "^${SCRIPT_WORKSPACE}" ; then
    echo "- Deleting existing workspace $SCRIPT_WORKSPACE"
    tf workspace -noprompt -delete -collection:"$COLLECTION" "$SCRIPT_WORKSPACE"
fi
echo "- Creating new workspace $SCRIPT_WORKSPACE"
tf workspace -noprompt -new -collection:"$COLLECTION" "$SCRIPT_WORKSPACE" -comment:"RegressionTestBench" -location:local
echo "- Mapping tfs folder $SCRIPT_TFSFOLDER in workspace $SCRIPT_WORKSPACE to folder $SCRIPT_DIRECTORY"
tf workfold  -noprompt -map -collection:"$COLLECTION" -workspace:"$SCRIPT_WORKSPACE" "$SCRIPT_TFSFOLDER" "$SCRIPT_DIRECTORY"

# creating directory to hold the scripts
echo "- (Re-)creating work folder $SCRIPT_DIRECTORY"
rm -rf "$SCRIPT_DIRECTORY"
mkdir -p "$SCRIPT_DIRECTORY"
cd "$SCRIPT_DIRECTORY"

echo "`pwd` - Retrieving regression test bench ..."
tf get -recursive -force "$SCRIPT_DIRECTORY"

echo "- Deleting workspace $SCRIPT_WORKSPACE"
tf workspace -noprompt -delete -collection:"$COLLECTION" "$SCRIPT_WORKSPACE"

