#!/bin/csh
#
# Generated by CMAKE from @CMAKE_CURRENT_SOURCE_DIR@/MoveInstalls.csh.cmake
#

set version = @BM_VERSION_STRING@
if ($# == 1) then
   set version = $1
endif
set installDirectory = @SSSDEV_INSTALL_DIR@
set installsToMove = "${version}_b ${version}_a ${version}"
set installToRemove = "${version}_c"
set installToMoveTo = $installToRemove

# Setting UMASK
umask 0002

if (-d $installDirectory/$installToRemove) then
      echo "$installDirectory/$installToRemove exists, removing it"
   rm -rf $installDirectory/$installToRemove
   echo
endif
foreach installToMove ($installsToMove)
   if (-d $installDirectory/$installToMove) then
      echo "$installDirectory/$installToMove exists"
      if (! -d  $installDirectory/$installToMoveTo) then
	 echo "$installDirectory/$installToMoveTo does not exist"
	 echo "moving $installDirectory/$installToMove to $installDirectory/$installToMoveTo"
	 mv $installDirectory/$installToMove $installDirectory/$installToMoveTo
      else
	 echo "$installDirectory/$installToMoveTo exists"
	 echo "Cannot move $installDirectory/$installToMove to $installDirectory/$installToMoveTo"
	 exit 1
      endif
   else
      echo "$installDirectory/$installToMove does not exist"
      echo "Nothing to move"
   endif
   set installToMoveTo = $installToMove
   echo
end
exit 0
