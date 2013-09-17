#########################################################################
#                                                                       #
# Copyright (C) 2012-2013 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

add_library( genex4_util
	src/gxexit.c src/gxggrs.c src/gxgloc.c src/gxgpar.c src/gxgsrs.c src/gxinit.c 
	src/gxkerr.c src/gxrun.c src/gxsloc.c src/gxspar.c src/ibsgetcnvrs.c 
	src/ibsgetexpapi.c src/ibsgetexpmc.c src/ibsgetexpr.c src/ibsgetgenmc.c 
	src/ibsgetgenr.c src/ibsgetvre.c src/ibsgxcfgprj.c src/ibsgxkernel.c 
	src/ibsutil.c
)

include_directories(../genex4_kernel/src)

install(TARGETS genex4_util
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

