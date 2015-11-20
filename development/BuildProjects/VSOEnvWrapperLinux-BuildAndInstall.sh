#!/bin/bash

# This script converts VSO build agent environment variables to the set of variables suitable for 
# Linux-BuildAndInstall.sh script and run this script

export SRC_DIR=$BUILD_SOURCESDIRECTORY
export BUILD_DIR=$BUILD_STAGINGDIRECTORY
export INSTALL_DIR=$BUILD_STAGINGDIRECTORY
export UNIT_TEST_OUTPUT_DIR=$BUILD_STAGINGDIRECTORY
export TFS_VERSION=$BUILD_SOURCEVERSION
export BUILD_NUMBER=$BUILD_BUILDNUMBER

# get full path to the current script
SELFPATH="$( cd "$(dirname "$0")" ; pwd -P )"

# clean build dir
echo "Cleaning build folder: ${BUILD_DIR}"
pushd ${BUILD_DIR}
rm -rf *
popd
ls -la "${BUILD_DIR}"

# call build and install script
$SELFPATH/Linux-BuildAndInstall.sh "$@"