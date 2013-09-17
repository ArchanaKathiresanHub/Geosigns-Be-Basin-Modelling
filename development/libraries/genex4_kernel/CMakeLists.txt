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

add_library(genex4_kernel
	src/error.c src/gxhist.c src/gxlabexp.c src/gxnumer.c src/gxrdfile.c 
	src/gxscanner.c src/list.c src/gxconfig.c src/gxhorizn.c src/gxlithol.c 
	src/gxoption.c src/gxressub.c src/gxsysdep.c src/gxerror.c src/gxinstnc.c 
	src/gxlocat.c src/gxphchem.c src/gxrock.c src/gxtrap.c src/mshell.c 
	src/gxfile.c src/gxkernel.c src/gxmodel.c src/gxprjct.c src/gxsbarea.c 
	src/gxutils.c src/sortlst.c
)

install(TARGETS genex4_kernel
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

