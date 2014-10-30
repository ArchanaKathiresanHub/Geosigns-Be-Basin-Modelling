#!/bin/bash

# Essential parameters
LIBS="Boost Eigen PETSC HDF5"
LIBDIR="/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc-lib-staging-`whoami`"
SRC=`dirname $0`/..
USERGROUP=g_psaz00

# User name check
whoami=`whoami`
if [ x$whoami != xs_bpac00 ]; then
  echo "Use the s_bpac00 user account to rebuild the libraries, because nobody else would be able to edit the directory"
  exit 1
fi


# Convert to absolute path names
pushd $SRC > /dev/null || { echo "Cannot find source directory $SRC"; exit 1; }
SRC=`pwd -P`
popd > /dev/null

mkdir -p $LIBDIR || { echo "Cannot create destination directory $LIBDIR"; exit 1; }
pushd $LIBDIR > /dev/null
LIBDIR=`pwd -P`
popd > /dev/null

# Build the libraries for every platform
for platform in "RHEL6.4" "RHEL5.8" "RHEL5.10"
do
   bash $SRC/BuildProjects/Linux-DoOnPlatform.sh $platform bash <<EOF
#!/bin/bash

TMPLIBDIR=\`mktemp -d\`
BUILDDIRS=

function onExit()
{
  # remove the temporary directory made for the build directory
  chmod -R u+w \$BUILDDIRS \$TMPLIBDIR
  rm -rf \$BUILDDIRS \$TMPLIBDIR
}
trap onExit EXIT


for IntelCompiler in ON OFF
do
   for IntelMPI in ON 
   do
      for sharedlib in ON OFF
      do 
         for config in Release Debug DebugAll MemCheck
         do
           BUILD=\`mktemp -d\`
           BUILDDIRS="\$BUILD \$BUILDDIRS"
           ( cd \$BUILD ; $SRC/bootstrap.csh -DCMAKE_BUILD_TYPE=\$config -DBUILD_SHARED_LIBS=\$sharedlib -DBM_USE_INTEL_COMPILER=\$IntelCompiler -DBM_USE_INTEL_MPI=\$IntelMPI -DBM_EXTERNAL_COMPONENTS_REBUILD=ON -DBM_EXTERNAL_COMPONENTS_DIR=\$TMPLIBDIR ; source envsetup.sh ; make -j16 $LIBS ; ) &

         done
      done
      wait
   done
done

wait

# Restrict the permissions again
echo -n "Marking all files as readable by everybody but read-only in the temporary library directory \$TMPLIBDIR ..."
chgrp -R $USERGROUP \$TMPLIBDIR
find \$TMPLIBDIR -type d -print0 | xargs -0 chmod a+x,a+r,a-w
find \$TMPLIBDIR -type f -print0 | xargs -0 chmod a+r,a-w
echo "DONE"

echo "Copying new library directory to destination $LIBDIR"
mkdir -p $LIBDIR
rsync -av \$TMPLIBDIR/ $LIBDIR
echo DONE
EOF

done

echo Libraries are ready for deployment in directory $LIBDIR
echo Please move them manually to the correct destination


