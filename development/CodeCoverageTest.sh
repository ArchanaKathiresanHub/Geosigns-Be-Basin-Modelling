#!/bin/bash

# This script performs a code coverage test, by comparing the test coverage of your local branch to that of the master branch.
# If the code coverage of your local branch is higher than or equal to that of master, the test passes

# Usage: Run the script by running ./CodeCoverageTest.sh -b <Path to code coverage build directory>
# If the <Path to code coverage build directory> does not exist, it is created automatically.
 
# First-time usage:
# If you want to create a new code coverage build directory, do this using this script by specifying
# the name of your new directory, but do not create an empty directory manually before running this script.

#Define branch location
BRANCH_LOCATION=$(pwd)

# Define functions
printHelp()
{
 echo ----------------------------------------------------------------
 echo Help:
 echo This script performs a code coverage test, by comparing the 
 echo coverage of your local branch to that of the master branch.
 echo If the code coverage of your local branch is higher than or 
 echo equal to that of master, the test passes.
 echo ""
 echo Usage: Run the script by running:
 echo ./CodeCoverageTest.sh -b [Path to code coverage build directory]
 echo ""
 echo If the [Path to code coverage build directory] does not exist, 
 echo it is created automatically.
 echo ""
 echo First-time usage:
 echo If you want to create a new code coverage build directory, 
 echo do this using this script by specifying the name of your new 
 echo directory, but do not create an empty directory manually before 
 echo running this script.
 echo ----------------------------------------------------------------
}

buildAndAnalyze()
{
  cd $BUILD_LOCATION
  make rebuild_cache
  $BRANCH_LOCATION/makeAll.sh
  make test
  profmerge
  codecov -spi pgopti.spi -dpi pgopti.dpi -xmlbcvrgfull BasinModelling_fcvrg.xml -counts -comp includedFiles.txt
}

moveResultsToFolder()
{
  rm -r $1
  mkdir $1
  mv CodeCoverage/ $1/
  mv CODE_COVERAGE.HTML $1/
}

excludeFromAnalysis()
{
  echo "~$1" >> $BUILD_LOCATION/includedFiles.txt
}

# Read input arguments
while getopts b: flag;
do
    case "${flag}" in
        b) BUILD_LOCATION=${OPTARG};;
    esac
done

# Check if build location is valid
if [ -z $BUILD_LOCATION ]
then
  echo ----------------------------------------------------------------
  echo "Aborted: Specify a build directory using the -b flag"
  printHelp
  exit
fi

if [ ! -d $BUILD_LOCATION ]
then
  echo "Making new Code Coverage build directory"
  mkdir $BUILD_LOCATION
  cd $BUILD_LOCATION
  $BRANCH_LOCATION/bootstrap.sh -DBM_CODE_COVERAGE_ENABLE=ON -DCMAKE_BUILD_TYPE=CodeCoverage
fi 

# Load necessary modules
cd $BRANCH_LOCATION
source setupEnv.sh
module load git

# Save the list of files which are changed with respect to master into a text file in the build directory
LOCALBRANCH_NAME=$(git branch --show-current)

if [[ $LOCALBRANCH_NAME == "master" ]]
then
  echo "Aborted: You are currently on master, please check out your development branch to make a code coverage comparison"
  exit
fi

gitStatusMessage=$(git status 2>&1)
if [[ $gitStatusMessage != *"nothing to commit, working tree clean" ]]
then
  echo "Aborted: Please stash or commit your local changes before running this script"
  exit
fi

rm $BUILD_LOCATION/includedFiles.txt
git diff --name-only master > $BUILD_LOCATION/includedFiles.txt
excludeFromAnalysis /test/ # exclude the test folders from the analysis
excludeFromAnalysis libraries/casaWizard/src/common/view 
excludeFromAnalysis libraries/casaWizard/src/common/control
excludeFromAnalysis libraries/casaWizard/src/sac/control
excludeFromAnalysis libraries/casaWizard/src/sac/view

# Install binaries and run the codecov tool using the text file as argument value for -comp 
buildAndAnalyze
moveResultsToFolder CC_DevelopmentBranch

cd $BRANCH_LOCATION
git checkout master
git pull

# Install binaries master and running analysis
buildAndAnalyze
moveResultsToFolder CC_MasterBranch

# Checkout the Dev branch again, such that the state is not changed after running the script
cd $BRANCH_LOCATION
git checkout $LOCALBRANCH_NAME

mapfile -t percentagesMaster < <(grep 'STYLE="font-weight:bold"' $BUILD_LOCATION/CC_MasterBranch/CodeCoverage/__CODE_COVERAGE.HTML | sed -e 's/^<TD ALIGN="center" STYLE="font-weight:bold"> //g' -e 's/<\/TD>//g')
mapfile -t percentagesDevelopment < <(grep 'STYLE="font-weight:bold"' $BUILD_LOCATION/CC_DevelopmentBranch/CodeCoverage/__CODE_COVERAGE.HTML | sed -e 's/^<TD ALIGN="center" STYLE="font-weight:bold"> //g' -e 's/<\/TD>//g')

COUNTER=0

for i in "${percentagesMaster[@]}"
do
  if echo "${percentagesDevelopment[$COUNTER]} $i -p" | dc | grep > /dev/null ^-; then
    tput setaf 1; 
    echo ""
    echo "-------------------------------------------------------------"
    echo "Failed: Code coverage of your development branch is lower than that of master. Please look at the code coverage results"
    echo "-------------------------------------------------------------"
    echo ""

    # Firefox cannot open with the current modules loaded
    module purge
    firefox $BUILD_LOCATION/CC_MasterBranch/CODE_COVERAGE.HTML &
    firefox $BUILD_LOCATION/CC_DevelopmentBranch/CODE_COVERAGE.HTML &
    tput sgr0
    exit
  fi
  COUNTER=$[COUNTER + 1]
done

tput setaf 2; 
echo ""
echo "-------------------------------------------------------------"
echo "Passed: Code coverage of your development branch is higher than or equal to that of master"
echo "-------------------------------------------------------------"
echo ""

tput sgr0




