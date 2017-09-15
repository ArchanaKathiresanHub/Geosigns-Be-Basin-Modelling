#!/bin/csh
set wd = `pwd`

pushd `dirname $0` > /dev/null
set source_directory = `pwd`
popd > /dev/null

set exit_status = 0

set yesterday=`date -d yesterday +"%F %T %z"`
set today=`date +"%F %T %z"`
set logoptions=""
set svnrepo="svn://rijkes-n-d99540.linux.shell.com"

set buildpath = /scratch/`whoami`/basinmodeling-cmake-build
set preinstallpath = /scratch/`whoami`/basinmodeling-cmake-install

if (! $?IBS_LIMITEDBUILDANDINSTALL ) then
	setenv IBS_LIMITEDBUILDANDINSTALL true
	setenv IBS_FULLBUILDANDINSTALL true
endif

# Build all applications: first parallel and then serial. The 'parallel' build will build everything
# and the 'serial' build only the serial applications. Therefore the order is important.
set build_success = 1

echo
echo Creating build directory: $buildpath
rm -rf $buildpath
mkdir -p $buildpath

pushd $buildpath
$source_directory/bootstrap.csh || exit 1

source envsetup.csh

echo Build applications
make -j 20 || set build_success = 0;

echo Move binaries to temporary installation directory: $preinstallpath
cmake . -DCMAKE_INSTALL_PREFIX=$preinstallpath
make -k install || set build_success = 0;

popd

if ( ! $build_success ) then
    echo failed to build applications
    set logoptions = "-v"
    set exit_status = 1
else
    echo succeeded to build applications
endif

exit $exit_status
