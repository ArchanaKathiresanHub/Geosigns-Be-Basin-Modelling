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

if(UNIX)

   set( CBM_HOME /nfs/rvl/groups/ept-sg/SWEast/Cauldron CACHE PATH "CBM path (affects HDF5, PETSC, etc.)")
   set( HPC_HOME ${CBM_HOME}/hpc)

   execute_process(
         COMMAND getos2
         OUTPUT_VARIABLE CSCE_FULL
         OUTPUT_STRIP_TRAILING_WHITESPACE
   )

   message(STATUS "CSCE OS Version identifier = ${CSCE_FULL}")
      
endif(UNIX)

if(WIN32)
   math(EXPR _64 "${CMAKE_SIZEOF_VOID_P} * 8")
endif()
	
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/../3rdparty)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
