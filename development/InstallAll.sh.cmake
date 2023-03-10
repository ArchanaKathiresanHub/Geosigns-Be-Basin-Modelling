#!/bin/bash

#
# Generated by CMAKE from @CMAKE_CURRENT_SOURCE_DIR@/InstallAll.csh.cmake
#

# This script installs the Cauldron applications in the final drop location,
# which is normally /apps/sssdev/ibs. When everything goes OK, the exit
# status is 0. In case there already exists a previous installation or if there
# is another failure, the exit status will be non zero.

# Exit immediately when any command exits with non-zero exit status
set -e

# search for the correct group
cldgrp=`groups 2> /dev/null` || true
if [[ "$cldgrp" =~ "g_psaz00" ]];then
   cldgrp="g_psaz00"
   installgrp="--group=$cldgrp"
else
   cldgrp=""
   installgrp=""
fi

# Check whether we don't add too many versions.
# The SSSLauncher (which many people still use) has a problem when the sum of
# all version string lengths is more than a 1000.
pushd @BM_SSSDEV_INSTALL_DIR@ > /dev/null
stringLength=$( echo @BM_VERSION_STRING@ v[0-9][0-9.]* | wc -c )
if [ $stringLength -gt 1000 ]; then
  echo "@BM_SSSDEV_INSTALL_DIR@ would have too many versions. Aborting installation"
  exit 1
fi
popd > /dev/null

# directory to put the release in
targetDirectory=@BM_SSSDEV_INSTALL_DIR@/@BM_VERSION_STRING@

# main system directory
mainSystemDirectory=@CSCE_PLATFORM@
# main system binary directory
mainBinaryDirectory=$targetDirectory/$mainSystemDirectory/bin
miscDirectory=$targetDirectory/misc

# Setting UMASK
umask 0002

if [ -d $mainBinaryDirectory ]; then
  echo "Main binary directory $mainBinaryDirectory already exists. Aborting installation..."
  exit 1
fi

#building the directory structure
echo "Making target directory $targetDirectory"
mkdir -p $mainBinaryDirectory
if [  "x${cldgrp}" != "x" ]; then
    echo "Setting correct permissions to $targetDirectory"
    chgrp -R $cldgrp ${targetDirectory}
    chmod g+rw ${targetDirectory}
    chmod g+s $targetDirectory
fi

# Install results of main build
echo "Installing binaries"
install $installgrp --mode=775 @CMAKE_INSTALL_PREFIX@/bin/* $mainBinaryDirectory

# BPA expects the binaries to be in the LinuxRHEL64 folder. For BPA we pick the RHEL64 binaries
pushd $targetDirectory > /dev/null
ln -s @CSCE_PLATFORM@ LinuxRHEL64
popd > /dev/null

# Install platform independent files
if [ -d $miscDirectory ]; then
  echo "Platform independent files are already installed. Skipping those..."
else
   echo "Installing platform independent files:"
   echo " - Geocase"
   # Install Geocase
   cp -r @CMAKE_INSTALL_PREFIX@/misc $targetDirectory

   if [ "x${cldgrp}" != "x" ]; then
      echo " - Changing group to $cldgrp"
      chgrp -R "${cldgrp}" $targetDirectory/misc
   fi

   echo " - Changing mode to g+w"
   chmod -R g+w $targetDirectory/misc
fi

echo "Configuring installation"
echo " - Marking installation as successful"
touch $miscDirectory/successfully_installed


echo "DONE"
