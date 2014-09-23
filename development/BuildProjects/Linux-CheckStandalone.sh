#!/bin/bash

# Reading parameters with some defaults
svnRepository=file:///nfs/rvl/groups/ept-sg/SWEast/Cauldron/SUBVERSION/repository
src=${SRC_DIR:-`dirname $0`/../..}
build=${BUILD_DIR:-`mktemp -d`}
installdir=${INSTALL_DIR:-"${build}"}
unit_test_output=${UNIT_TEST_OUTPUT_DIR:-"${build}"}
platform=${PLATFORM:-Linux}
configuration=${CONFIGURATION:-Release}
nprocs=${NUMBER_OF_CORES:-20}
version_number_major=${VERSION_NUMBER_MAJOR:-`date +%Y`}  
version_number_minor=${VERSION_NUMBER_MINOR:-`date +%m`}  
version_tag=${VERSION_TAG:-`whoami`}
remote_host=${REMOTEHOST:-houcy1-n-sp030b01.americas.shell.com}
build_number=${BUILD_NUMBER:-XXXX}

# Make some temporary files to scripts in 
run_on_cluster=`mktemp`
script=`mktemp`

# Set some code to execute when shell script exits
function onExit()
{
  # remove the temporary directory made for the build directory
  if [ "x${BUILD_DIR}" = x ]; then
     rm -Rf $build
  fi
  
  rm -f $run_on_cluster $script
}

trap onExit EXIT

# Test the build directory
test -d "$build" || { echo "Given build directory '$build' does not exist"; exit 1; }
ls "$build"/* > /dev/null 2>&1 && { echo "Given build directory '$build' is not empty"; exit 1; }


# set UMASK
umask 0002

# set dirs as absolute paths
pushd $src
src="`pwd -P`"
popd

pushd $build
build="`pwd -P`"
popd

pushd $installdir
installdir="`pwd -P`"
popd

pushd $unit_test_output
unit_test_output="`pwd -P`"
popd

# Standard applications
CMAKE=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/cmake/cmake-2.8.10.2/Linux64x_26/bin/cmake
CTEST=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/Tools/cmake/cmake-2.8.10.2/Linux64x_26/bin/ctest

# Package Cauldron
pushd $build
echo Packaging cauldron source
${CMAKE} "-DCPACK_PACKAGE_VERSION=${build_number}" ${src} || { echo "Could not configure source package" ; exit 1; }
make package_source                                       || { echo "Could not make source package" ; exit 1; }
local_host=`hostname --fqdn`                              || { echo "Could not get hostname" ; exit 1; }
tar=BasinModeling-${build_number}-Source.tar.gz
tarfile=$build/$tar

# Make a script to run something on HOUGDC cluster

cat > $run_on_cluster <<"EOF"
#!/bin/bash
source /glb/apps/hpc/lsfprod/conf/profile.lsf
bsub -P cldrn -Is -q default.q "$@"
EOF


# Make script to be executed on the remote host
cat > $script <<EOF
#!/bin/bash

local_host=${local_host}
tarfile=${tarfile}
procs=${nprocs}
unit_test_output=${unit_test_output}
EOF

cat >> $script <<"EOF"

remoteBuildDir=`mktemp -d` || { echo Could not create build directory; exit 1; }

function onExit()
{
  # remove the temporary directory made for the build directory
  if [ "x${remoteBuildDir}" = x ]; then
     rm -Rf ${remoteBuildDir}
  fi
}

trap onExit EXIT

CMAKE=/glb/data/ifshou_ird02/projects/cauldron/cmake/3.0.2/bin/cmake
MPIROOT=/apps/3rdparty/intel/impi/4.1.3.049
MKLROOT=/apps/3rdparty/intel/parallel_studio_xe/mkl

tar=`basename $tarfile`
srcdir=`basename $tarfile .tar.gz`

pushd ${remoteBuildDir}

echo "Copying the source package to the remote host"
scp -q -o StrictHostKeyChecking=no -o CheckHostIP=no ${local_host}:${tarfile} . || { echo Could not copy source archive; exit 1; }

echo "Untarring the source package"
tar xzvf $tar || { echo Could not uncompress tarfile; exit 1 ; }

echo "Configuring the package"
${CMAKE} \
   -DBUILD_SHARED_LIBS=ON \
   -DBM_USE_INTEL_MPI=ON \
   -DINTEL_MPI_ROOT=$MPIROOT \
   -DINTEL_MKL_ROOT=$MKLROOT \
   -DBM_UNIT_TEST_OUTPUT_DIR=. \
   -DBM_CONFIG_PRESET=OFF \
   ${srcdir}/development \
   || { echo "Configuration of standalone package has failed" ; exit 1; }

source envsetup.sh

echo "Building the package"
make -j${procs} -k \
   || { echo "Build of standalone package has failed"; exit 1; }

echo "Testing the package"
ctest \
   || { echo "One or more unit tests have failed" ; exit 1 ; }

scp *-junit.xml ${local_host}:${unit_test_output}

popd
EOF

SSH_OPTS="-q -o StrictHostKeyChecking=no -o CheckHostIP=no"
echo "Copy script to remote host"
scp $SSH_OPTS ${run_on_cluster} ${remote_host}:${run_on_cluster} || { echo "Could not copy the script to the remote build host"; exit 1; }

echo "Execute the script on the remote host"
exit_status=0
ssh $SSH_OPTS  ${remote_host} /bin/bash ${run_on_cluster} < ${script} 
if [ $? != 0 ]; then
  echo "error: Standalone version could not be compiled"
  exit_status=1
fi

echo "Installing generated tarball"
if [ "x$installdir" != "$build" ]; then
   cp $tarfile $installdir
fi

echo "Cleanup"
ssh $SSH_OPTS  ${remote_host} rm -f $run_on_cluster

exit $exit_status
