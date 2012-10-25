#!/bin/csh
# ----------------------------------------------------------------------------
# goibs_startup_2.login
# Set environment variables for IBS environment that are independent of 
# the version (production, test or develop).
# ----------------------------------------------------------------------------
 
#
# CAULDRON version number. Must always point to the latest debug version !
#                          Used for firing the correct versions of 
#                          grid2grid etc from cauldron_test !
#
setenv CAULDRON_VERSION v2006.09
#
# Open Inventor version
#
setenv OIV_VERSION /apps/3rdparty/oiv/5.0
setenv OIVHOME ${OIV_VERSION}
#
#  Platform path of executable system:
#  ===================================
# General library versions (needed for old/new 32 or 64 bits on Silicon)

#
# resolve sss / oss platform names
#
setenv APPSOS `/apps/oss/etc/getos`

switch (`uname`)
  case Linux
#
# Linux 64 / 32 bits:
#
    if ${?USE64BITS} then
       setenv IBS_SYSTEM $IBS_HOME/Linux64	# 64 bits OS 
       setenv machine 'Linux64'
       setenv PETSC_ARCH linux64
       setenv GOSVER g
       setenv OIV_HOME $OIV_VERSION/Linux64
    else
       setenv IBS_SYSTEM $IBS_HOME/Linux32	# 32 bits OS 
       setenv machine 'Linux32'
       setenv PETSC_ARCH linux
       setenv GOSVER g
       setenv OIV_HOME $OIV_VERSION/Linux
    endif
    breaksw
case IRIX64:
#
#  Irix 64 / 32 bits
#
    if ${?USE64BITS} then
       setenv IBS_SYSTEM $IBS_HOME/SG64	# 64 bits OS 
       setenv machine 'Silicon64'
       setenv PETSC_ARCH IRIX64
       setenv GOSVER n64
       setenv OIV_HOME $OIV_VERSION/IRIX64
     else
       setenv IBS_SYSTEM $IBS_HOME/SG32	# new 32 bits OS 
       setenv machine 'Silicon32'
       setenv PETSC_ARCH IRIX
       setenv GOSVER n32n
       setenv OIV_HOME $OIV_VERSION/IRIX
     endif
    breaksw
case SunOS:
#
#  Sun 64 / 32 bits
#
    if ${?USE64BITS} then
       setenv IBS_SYSTEM $IBS_HOME/SunOS64   # 64 bits OS 
       setenv machine 'SunOS64'
       setenv PETSC_ARCH solaris64
       setenv GOSVER n64
       setenv OIV_HOME $OIV_VERSION/SunOS64
    else
       setenv IBS_SYSTEM $IBS_HOME/SUN	     # 32 bits OS 
       setenv machine 'Sun'
       setenv PETSC_ARCH solaris
       setenv GOSVER n
       setenv OIV_HOME $OIV_VERSION/SunOS
    endif
    breaksw
endsw

#  If IBS_VERSION is not set then set it to the current production version:
   if ${IBS_VERSION} == '' then
      setenv IBS_VERSION `cat $IBS_HOME/bin/SO_CURRENT_PRODUCTION_VERSION`
   endif

#  Set env variable to IBS reference data files (= the IBS "misc" directory):
   setenv IBS_NAMES $IBS_SYSTEM/$IBS_VERSION/misc

#  Set env variable to platform dependent pvtpack/genex4 reference data files
   setenv IBS_NAMES_PLATFORM $IBS_NAMES"/"`uname -s`

#  Set env variable to GOSPEL applications reference files
#       (= the IBS "misc" directory):
   setenv GOSPELRTE $IBS_NAMES 

#
#  Set up the development environment
#
   setenv IBS_SYSTEM /nfs/rvl/users/ibs/${machine}
   setenv IBS_NAMES  $IBS_SYSTEM/$IBS_VERSION/misc
   setenv soexdir    $IBS_SYSTEM/$IBS_VERSION/exe
   setenv soardir    $IBS_SYSTEM/$IBS_VERSION/obj

#  Set env variable for path to the GOSPEL script files
#       (= under the IBS "misc" directory):
   if ${?SCRPATH} == 0 then
      setenv SCRPATH $IBS_NAMES/GospelScriptFiles
   else
      setenv SCRPATH $IBS_NAMES/GospelScriptFiles:$SCRPATH 
   endif

#  Set env variable to genex3 data files:
   setenv GENEXDIR $IBS_SYSTEM/$IBS_VERSION/misc/genex40/

#  Set env variable to Sepran environment:
   setenv SPHOME $IBS_HOME/IBS/EXTERNALS/SEPRAN/curr

#  Set environment for GOCAD linking.
   source $IBS_HOME/bin/sogocadenvironment

#  Set environment for HDF5 linking.
   source $IBS_HOME/bin/sohdf5environment

#  Setting for OPEN INVENTOR error suppression / black swapping screens
#  Font PATH too

   setenv OIV_NO_OVERLAYS aaa
   setenv OIV_PSFONT_PATH ${OIVHOME}
 
#  Set path for flex lm includes and libraries
   setenv FLEXLMINCLUDEDIR /apps/3rdparty/EPTFlexLm/v9.2

#  Set path through executable directory: Development version...
   set path = (/apps/sssdev/share $path)

#  Set variable for X resources to include the IBS "misc" directory:
   if ${?XUSERFILESEARCHPATH} == 0 then
      setenv XUSERFILESEARCHPATH $IBS_NAMES/%N
   else 
      setenv XUSERFILESEARCHPATH $XUSERFILESEARCHPATH':'$IBS_NAMES/%N
   endif
#  just set default
   if ${?LD_LIBRARY_PATH} == 0 then
      setenv LD_LIBRARY_PATH /usr/lib
   endif


#  All done
#  --------
