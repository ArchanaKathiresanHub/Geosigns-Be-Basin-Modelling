#!/bin/csh -f
#
##
#
# Setup current Geocase environment
# Change versions here in the next 2 lines....
#
setenv GHOME ${IBS_HOME}/geocase_64
setenv GEO_CURRENT_VERSION 2.4
setenv BasinModCaseVersion "BasinModCase"2.4
setenv FIGLETHOME ${IBS_HOME}/FIGLET/figlet222/${machine}
setenv GeoCaseVersion      "GeoCaseVersion"$GEO_CURRENT_VERSION
 
if ${?USE64BITS} then
  ${FIGLETHOME}/figlet "$machine-64 bits"
else
  ${FIGLETHOME}/figlet "$machine-32 bits"
endif
echo "Local directory: "$LOCAL
echo "Machine        : "`uname -n`
echo "Platform       : "$machine

setenv  GEOCASE $GHOME/prod/v$GEO_CURRENT_VERSION
source $GEOCASE/bin/geocase.login
source $GEOCASE/bin/geocase.alias.csh

#
# FlexLM (temporary) license file setting.
# remove the /var/flexlm/license.dat when oss is ok
# ========================================
source ${IBS_HOME}/bin/golicense
#
###if ${?LM_LICENSE_FILE} == 0 then
###setenv LM_LICENSE_FILE 3001@AMSDC1-S-768.europe.shell.com':'3001@CBJ-S-459.asia-pac.shell.com':'3001@HOUIC-S-438.americas.shell.com':'SIEPRTS_LICENSE_FILE=3000@AMSDC1-S-768.europe.shell.com':'3000@CBJ-S-459.asia-pac.shell.com':'3000@HOUIC-S-438.americas.shell.com 
##setenv LM_LICENSE_FILE /apps/3rdparty/etc/licenses/license.dat':'/var/flexlm/license.dat
###else 
## setenv LM_LICENSE_FILE $LM_LICENSE_FILE':'/apps/3rdparty/etc/licenses/license.dat':'/var/flexlm/license.dat
###setenv LM_LICENSE_FILE $LM_LICENSE_FILE':'3001@AMSDC1-S-768.europe.shell.com':'3001@CBJ-S-459.asia-pac.shell.com':'3001@HOUIC-S-438.americas.shell.com':'SIEPRTS_LICENSE_FILE=3000@AMSDC1-S-768.europe.shell.com':'3000@CBJ-S-459.asia-pac.shell.com':'3000@HOUIC-S-438.americas.shell.com 
###endif

setenv LM_LICENSE_FILE $LM_LICENSE_FILE':'/apps/3rdparty/etc/licenses/license.dat':'/var/flexlm/license.dat
#
# Next line needed for local development. Please note that the variable is set by ssss for the released version.
# ==============================================================================================================
setenv LM_PROJECT SIEP_BV
#
# TGS FlexLM (temporary) license file setting.
# changed because of apps problems. TGS likes only one file..
# ===========================================
 setenv TGS_LICENSE_FILE $IBS_NAMES/.tgs_password.dat
#
# test scripts work with oss mgdiff on all platforms..
#
source /apps/oss/etc/oss.login
setenv PATH /apps/oss/bin:$PATH
#
# template initialisations / prelinking stage
#
setenv LD_LIBRARY_PATH /usr/lib32/cmplrs:$LD_LIBRARY_PATH
#
#  setenv XUSERFILESEARCHPATH /apps/oss/lib/app-defaults/Mgdiff/%N%S:$XUSERFILESEARCHPATH

#
# Environment setup: $LOCAL must have been set...
# -----------------------------------------------
####  setenv OIVHOME /apps/3rdparty/oiv/3.1.1

if ${?LOCAL} == 0 then
    echo " "
    echo " "
    echo " "
    echo " Environment variable LOCAL is not set."
    echo " Please set it to the top of your backbone and login again...."
    echo " "
    echo " "
else
   setenv SCRPATH ${LOCAL}/scr:${SCRPATH}
   setenv LD_LIBRARY_PATH ${LOCAL}/lib/${machine}:${LD_LIBRARY_PATH}
   setenv TMPDIR $LOCAL
#
#  Platform dependent settings
#
   switch (`uname -s`)
#
# Octane 64 bits.... 
#
   case IRIX64:
      setenv LD_LIBRARYN32_PATH ${LOCAL}/lib/${machine}:$LD_LIBRARYN32_PATH
      if ${?USE64BITS} then
        setenv LD_LIBRARY64_PATH ${LOCAL}/lib/${machine}:$LD_LIBRARY64_PATH
        setenv LD_LIBRARY64_PATH ${OIV_HOME}/lib:/usr/Motif-2.1/lib64:${LD_LIBRARY64_PATH}
        setenv GENEXBITS 64
      else
        setenv LD_LIBRARYN32_PATH ${OIV_HOME}/lib:$LD_LIBRARYN32_PATH
        setenv LD_LIBRARYN32_PATH ${LOCAL}/lib/${machine}:$LD_LIBRARYN32_PATH
        setenv GENEXBITS ""
      endif
      ${IBS_HOME}/cookie/cookie
      breaksw
#
# Octane new 32.... 
#
   case IRIX:
      setenv LD_LIBRARYN32_PATH ${LOCAL}/lib/${machine}:$LD_LIBRARYN32_PATH
      setenv LD_LIBRARYN32_PATH ${OIV_HOME}/lib:$LD_LIBRARYN32_PATH
      ${IBS_HOME}/cookie/cookie
      breaksw
#
# Sun.... 
#
   case SunOS:
      set path=(/usr/bin /usr/ucb /apps/oss/bin /usr/ccs/bin $path) 
      if ${?USE64BITS} then
        setenv LD_LIBRARY_PATH ${OIV_HOME}/lib:$LD_LIBRARY_PATH
        setenv LD_LIBRARY_PATH ${LOCAL}/lib/${machine}:/usr/openwin/lib/sparcv9:$LD_LIBRARY_PATH
        setenv GENEXBITS 64
        setenv PETSC_ARCH solaris64
      else
        setenv LD_LIBRARY_PATH ${OIV_HOME}/lib:$LD_LIBRARY_PATH
        setenv LD_LIBRARY_PATH ${LOCAL}/lib/${machine}:$LD_LIBRARY_PATH
        setenv GENEXBITS ""
      endif
      breaksw
   endsw
endif

