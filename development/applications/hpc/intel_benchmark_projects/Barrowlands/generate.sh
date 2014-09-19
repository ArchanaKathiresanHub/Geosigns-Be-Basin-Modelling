#!/bin/bash

script=`mktemp`

for size in micro small
do
  for difficulty in easy medium hard salt
  do

cat > $script <<EOF
s/@STRATIGRAPHY@//
T AfterInsertStrat
r template/${difficulty}-strat

: AfterInsertStrat
s/@DIMENSIONS@//
T AfterInsertDim
r template/${size}-dim

: AfterInsertDim
p
EOF

      name=$size-$difficulty
      mkdir -p $name
      sed -nf $script < template/Project.project3d  > $name/Project.project3d
      cp template/Inputs.HDF $name
   done
done

rm -f $script
