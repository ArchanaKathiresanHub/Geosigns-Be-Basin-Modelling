#!/bin/bash -xl

# Essential parameters
LIBS="Boost PETSC HDF5 Eigen"
SRC=`dirname $0`/..

if [ `hostname -s` == "okapi" ]; then
    USERGROUP='okapi_bm_dev'
    LIBDIR="/glb/data/cauldron/hpc-library"
    CPUS=24
else
    USERGROUP=g_psaz00
    LIBDIR="/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc-library"
    CPUS=5
fi

# clean up on exit
trash=
function onExit()
{
   rm -rf $trash
}

trap onExit EXIT

# User name check
whoami=`whoami`
if [ "x$whoami" != "xs_bpac00" ] && [ "x$whoami" != "xcoebs.basinmodelling" ]; then
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


# Precompiled combinations
combinations=`mktemp`
trash="$combinations $trash"
cat > $combinations  <<EOF
#OS      Intel Compiler   Intel MPI   Shared Libs   Configuration
#RHEL6.6  ON               ON          OFF           Release
#RHEL6.6  ON               ON          OFF           Debug
#RHEL6.6  ON               ON          ON            Release
#RHEL6.6  ON               ON          ON            Debug
#RHEL6.6  OFF              ON          OFF           Release
#RHEL6.6  OFF              ON          OFF           Debug
#RHEL6.6  OFF              ON          ON            Release
#RHEL6.6  OFF              ON          ON            Debug
#RHEL6.6  OFF              ON          ON            MemCheck
RHEL6.8  ON               ON          OFF           Release
RHEL6.8  ON               ON          OFF           Debug
RHEL6.8  ON               ON          ON            Release
RHEL6.8  ON               ON          ON            Debug
RHEL6.8  OFF              ON          OFF           Release
RHEL6.8  OFF              ON          OFF           Debug
RHEL6.8  OFF              ON          ON            Release
RHEL6.8  OFF              ON          ON            Debug
RHEL6.8  OFF              ON          ON            MemCheck
#RHEL7.2  ON               ON          OFF           Release
#RHEL7.2  ON               ON          OFF           Debug
#RHEL7.2  ON               ON          ON            Release
#RHEL7.2  ON               ON          ON            Debug
#RHEL7.2  OFF              ON          OFF           Release
#RHEL7.2  OFF              ON          OFF           Debug
#RHEL7.2  OFF              ON          ON            Release
#RHEL7.2  OFF              ON          ON            Debug
#RHEL7.2  OFF              ON          ON            MemCheck
EOF


# Build the libraries for every platform
for flavour in `awk '/^[^#]/ { printf "%s,%s,%s,%s,%s\n", $1, $2, $3, $4, $5 }' $combinations`
do
   platform=`echo $flavour | cut -f 1 -d ,`
   intel_compiler=`echo $flavour | cut -f 2 -d ,`
   intel_mpi=`echo $flavour | cut -f 3 -d ,`
   shared_libs=`echo $flavour | cut -f 4 -d ,`
   configuration=`echo $flavour | cut -f 5 -d ,`

   echo  'Run build for $platform intel_compiler: $intel_compiler'

#   bash $SRC/BuildProjects/Linux-DoOnPlatform.sh $platform bash -s <<EOF &
   cat >  test.sh <<EOF
#!/bin/bash

TMPLIBDIR=\`mktemp -d\` || { echo "Cannot create temporary installation directory"; exit 1; }
BUILD=\`mktemp -d\` || { echo "Cannot create temporary build directory"; exit 1; }

function onExit()
{
  # remove the temporary directory made for the build directory
  chmod -R u+w \$BUILD \$TMPLIBDIR
  rm -rf \$BUILD \$TMPLIBDIR
}
trap onExit EXIT


cd \$BUILD
$SRC/bootstrap.sh -DCMAKE_BUILD_TYPE=$configuration -DBUILD_SHARED_LIBS=$shared_libs -DBM_USE_INTEL_COMPILER=$intel_compiler -DBM_USE_INTEL_MPI=$intel_mpi -DBM_EXTERNAL_COMPONENTS_REBUILD=ON -DBM_EXTERNAL_COMPONENTS_DIR=\$TMPLIBDIR 
source envsetup.sh
make -j$CPUS $LIBS


# Restrict the permissions again
echo -n "Marking all files as readable by everybody but read-only in the temporary library directory \$TMPLIBDIR ..."
chgrp -R "$USERGROUP" \$TMPLIBDIR
find \$TMPLIBDIR -type d -print0 | xargs -0 chmod a+x,a+r,a-w
find \$TMPLIBDIR -type f -print0 | xargs -0 chmod a+r,a-w
echo "DONE"

echo "Copying new library directory to destination $LIBDIR"
mkdir -p $LIBDIR
rsync -av \$TMPLIBDIR/ $LIBDIR
echo DONE

exit 0
EOF

    if [ `hostname -s` == "okapi" ]; then
        bash $SRC/BuildProjects/test.sh
    else
        bash $SRC/BuildProjects/Linux-DoOnPlatform.sh $platform bash $SRC/BuildProjects/test.sh
    fi

done

rm -rf $SRC/BuildProjects/test.sh
wait

echo Libraries are ready for deployment in directory $LIBDIR
echo Please move them manually to the correct destination


