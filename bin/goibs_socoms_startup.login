#!/bin/csh
######################################################################
# goibs_socoms_startup: Set environment variables for IBS developers.  #
######################################################################
# set all hard references, use later

setenv afsdir    ${IBS_HOME}                    # afs directory
setenv sosysdir  ${IBS_HOME}		        # top directory SOCOMS
setenv sorcstree ${IBS_HOME}/RCS                # topdir. of the RCS-tree
setenv sorcslist ${IBS_HOME}/RCS/Admi/RCS_list	# list of all RCS files
setenv sobindir  ${IBS_HOME}/bin                # location for SOCOMS scripts
setenv sodevlvers 26				# development version number
                                                # THIS ONE TO BE CHANGED FOR
						# A NEW RELEASE !
##############################
# set version dependent items #
##############################

switch ("$ibsversion")
case Production:
						# set RCS CODE version number
						# 
						# NOTE:::NOTE:::NOTE:::NOTE:::
						# The soversion number must be
    setenv soversion 25				# one less then the sodevlvers
						# which is set above.
						# 
						# The soget, sofresh functions
						# (applied for 'production') 
						# will now retrieve the LAST
						# version for a new release.

    setenv soversdir `cat $sobindir/SO_CURRENT_PRODUCTION_VERSION` # set version DIRECTORY
    breaksw
case Test:
    setenv soversion $sodevlvers		# set RCS CODE version number
    setenv soversdir t				# set version DIRECTORY
    breaksw
case Development:
    setenv soversion $sodevlvers		# set RCS CODE version number
    setenv soversdir d				# set version DIRECTORY
    breaksw
default
    echo "Wrong IBS version set."
endsw

############
# set them #
############

setenv sosrcdir  ${IBS_HOME}/$soversdir		# version source directory
setenv somakedir ${IBS_HOME}/$soversdir/EPT-HM      # central make directory
setenv soardir   $IBS_SYSTEM/$soversdir/obj	# version archive directory
setenv soexdir   $IBS_SYSTEM/$soversdir/exe	# version exec. directory

switch (`whoami`)				# set user type
case ibs:					# important person
    setenv souser 'Maintainer'
    breaksw
default:					# other important persons
    setenv souser 'Developer'
    breaksw
endsw

set path=(. $sobindir $path)		# change path
