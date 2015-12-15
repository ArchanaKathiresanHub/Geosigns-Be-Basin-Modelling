#!/bin/bash

script=`mktemp`

# Generate Medium and Large sizes

for size in medium large
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

# Generate Dream and Impossible sizes
tmpdir=`mktemp -d`
cauldron_crop=/apps/sssdev/ibs/v2014.0703/LinuxRHEL64/bin/cauldron-crop
for size in 3 17 
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
r template/large-dim

: AfterInsertDim
p
EOF

      case $size in 
         3) name=dream-$difficulty
            ;;
         17) name=impossible-$difficulty
            ;;
         *) echo Unrecognized size; exit 1;
            ;;
      esac
      mkdir -p $name
      dest=`pwd`/$name


      # Set the difficulty of the project and save it in $tmpdir
      sed -nf $script < template/Project.project3d  > $tmpdir/Project.project3d
      cp template/Inputs.HDF $tmpdir

      # Super sample the grid
      pushd $tmpdir > /dev/null
      $cauldron_crop -oversample $size $size Project.project3d $dest
      popd > /dev/null

   done
done


rm -rf $script $tmpdir
