#!/bin/bash
#
groupid=g_elfen0

echo "$0 - creates a user directory with the correct permissions for sharing." 
echo ""


basedir=`dirname $0`
echo "Moving into [$basedir]."
cd $basedir
if [ $? -ne 0 ]; then
	echo "Move failed, exitting..."
	exit 1
fi

subdir=`whoami`
echo "Creating user directory [$subdir]."
mkdir $subdir
if [ $? -ne 0 ]; then
	echo "Make directory failed, exitting..."
	exit 1
fi

echo "Changing directory [$subdir] to group [$groupid]."
chgrp $groupid $subdir
if [ $? -ne 0 ]; then
	echo "Change group failed, exitting..."
	exit 1
fi

chmod 770 $subdir
if [ $? -ne 0 ]; then
	echo "Change mode of [$subdir] to 770 failed, exitting..."
	exit 1
fi
chmod g+s $subdir
if [ $? -ne 0 ]; then
	echo "Change mode of [$subdir] to set groupid failed, exitting..."
	exit 1
fi

echo ""
echo "Procedure completed successfully."
exit