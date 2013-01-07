#!/bin/bash

dir=`dirname $0`

if [ x$1 = xquiet ]; then
  export QUIET=true
fi

# Copy files temporarily
tmpdir=`mktemp -d` || { echo Could not create temporary directory; exit 2; }
cp -R $dir/input $tmpdir

# Check each file
all_valid=true
for f in `find $tmpdir -name "*.project3d"`
do 
  $dir/tools/check_project $f || { all_valid=false;  }
done

# remove temporary copy
rm -Rf $tmpdir

if [ x$all_valid = xtrue ]; then
  exit 0
else
  exit 1
fi
