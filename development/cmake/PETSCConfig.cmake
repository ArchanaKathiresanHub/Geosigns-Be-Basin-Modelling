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

set(PETSC_VERSION "3.4.1-intel13" CACHE STRING "PETSC Version")

set(PETSC_RELEASE_HOME ${HPC_HOME}/petsc/${PETSC_VERSION}/LinuxRHEL64_x86_64_58WS)
set(PETSC_DEBUG_HOME ${HPC_HOME}/petsc-debug/${PETSC_VERSION}/LinuxRHEL64_x86_64_58WS)
set(PETSC_HOME)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
   set(PETSC_HOME "${PETSC_RELEASE_HOME}")
else()
   set(PETSC_HOME "${PETSC_DEBUG_HOME}")
endif()

set(PETSC_FOUND TRUE)
set(PETSC_ROOT "${PETSC_HOME}" CACHE PATH "Installation directory of PETSc")
set(PETSC_INCLUDE_DIRS "${PETSC_ROOT}/include")
set(PETSC_LIBRARIES "${PETSC_ROOT}/lib/libpetsc.a")
set(PETSC_LINK_FLAGS "${MPI_LINK_FLAGS} ${OpenMP_CXX_FLAGS}")

add_external_package_info( 
    CAPABILITY  PETScLib
    NAME         "PETSc"
    VENDOR       "Argonne National Laboratory"
    VERSION      "3.4.1"
    LICENSE_TYPE "Simplified BSD"
    LICENSE_FILE "${THIRD_PARTY_DIR}/licenses/Petsc-3.4.1.txt"
    URL          "http://www.mcs.anl.gov/petsc/"
    DESCRIPTION  "Portable, Extensible Toolkit for Scientific Computation"
    REQUIRED_AT  "Runtime"
    COUNTRY_OF_ORIGIN "USA"
    SHIPPED      "Yes"
    INCLUSION_TYPE "Static Link"
    USEABLE_STAND_ALONE "No"
    CONTAINS_CRYPTO "No"
    ECCN         "EAR99"
)                   


# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
