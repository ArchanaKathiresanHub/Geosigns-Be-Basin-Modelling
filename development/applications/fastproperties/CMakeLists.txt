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

#FILE( GLOB all_headers src/*.h )
#FILE( GLOB all_srcs src/*.C)
set (all_headers,
   src/PropertiesCalculator.h)

set(all_srcs
   src/PropertiesCalculator.C
   src/main.C)

source_group(include FILES ${all_headers})

source_group(source FILES ${all_srcs})

add_executable(fastproperties
   ${all_srcs}
   ${all_headers}
)

bm_include_libraries(
   DataModel
   DataAccess
   DistributedDataAccess
   Interpolation
   LinearAlgebra
   GeoPhysics
   DerivedProperties
   DataAccess   
   TableIO
   CBMGenerics
   Serial_Hdf5
   Parallel_Hdf5
   EosPack
   utilities
   FileSystem
   Utilities_Petsc
   VisualizationIO
   VisualizationIO_projectHandle
)

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
   ${MPI_INCLUDE_DIRS}
   ${PETSC_INCLUDE_DIRS}
   ${PUGIXML_INCLUDE_DIR}
   ${LZ_INCLUDE_DIR}
)

target_link_libraries(fastproperties
   DerivedProperties
   GeoPhysics
   DataAccess
   DistributedDataAccess
   Interpolation
   LinearAlgebra
   TableIO
   EosPack
   CBMGenerics
   Serial_Hdf5
   Parallel_Hdf5
   utilities
   FileSystem
   VisualizationIO
   VisualizationIO_projectHandle
   PugiXMLlib
   LZlib
   ${HDF5_LIBRARIES}
   ${PETSC_LIBRARIES}
)

install(TARGETS fastproperties
   RUNTIME DESTINATION bin
)

generate_dox( fastproperties.cfg )

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
