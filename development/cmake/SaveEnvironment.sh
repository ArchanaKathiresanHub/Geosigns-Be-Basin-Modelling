#!/bin/bash

# Source all files given on the command line in order to set-up our environment
for param
do
   source $param
done

# Copy environment to file
environment=`mktemp`
set  > $environment

tmp=`mktemp`
trap 'rm $environment $tmp' EXIT

# Get list of read-only variables
readOnlyVars=$(readonly | sed -ne 's/^.*\s\([[:alnum:]_]\+\)=.*/\1/p' )

# Remove read-only variables from file
for var in $readOnlyVars
do
   sed -ne "/^${var}/d;p" $environment > $tmp
   cp $tmp $environment
done

# Output the environment to stdout
# Mark all subsequently set variables to be exported to the environment
echo 'set -a'
cat $environment

