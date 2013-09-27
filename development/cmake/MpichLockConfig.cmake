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

include(cmake/AddPackage.cmake)

add_external_package_info(
      CAPABILITY MpichLock
      NAME    "MPICH2"
      VENDOR  "Argonne National Laboratory"
      VERSION "1.4.1p1"
      LICENSE_TYPE "MIT"
# see http://lists.mcs.anl.gov/pipermail/mpich-discuss/2011-June/010206.html      
      LICENSE_FILE "/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/mpich2-1.4.1p1/COPYRIGHT"
      URL "http://www.mcs.anl.gov/project/mpich-high-performance-portable-implementation-mpi"
      DESCRIPTION "A High performance Portable Implementation of MPI"
      REQUIRED_AT  "Runtime"
      COUNTRY_OF_ORIGIN "USA"
      SHIPPED      "Yes"
      INCLUSION_TYPE "Static Link"
      USEABLE_STAND_ALONE "No"
      CONTAINS_CRYPTO "No"
      ECCN         "Unknown"
)
      

add_subdirectory(libraries/mpich2_lock)
set(BM_MPI_LOCK_LIBRARIES)
list(APPEND BM_MPI_LOCK_LIBRARIES "-Wl,--whole-archive" mpich2_lock "-Wl,--no-whole-archive")

