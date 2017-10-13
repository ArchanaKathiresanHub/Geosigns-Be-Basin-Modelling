#########################################################################
#                                                                       #
# Copyright (C) 2012-2015 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

if (BM_PARALLEL)

set( all_headers
   src/CrustalThicknessCalculator.h
   src/CrustalThicknessCalculatorFactory.h
)

set( all_srcs
   src/CrustalThicknessCalculator.C 
   src/CrustalThicknessCalculatorFactory.C 
   src/main.C
)

source_group(include FILES ${all_headers})
source_group(source FILES ${all_srcs})

set( APP_NAME "fastctc" )
add_executable( ${APP_NAME} ${all_srcs} ${all_headers} )
set_target_properties( ${APP_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${APP_NAME}" )

set_target_properties( ${APP_NAME} PROPERTIES SUFFIX ".exe" )

create_application_run_script( ${APP_NAME} )

application_env_var( TARGET ${APP_NAME} VARS
                     "CTCDIR" "$MISCDIR"
)

bm_include_libraries(
   DataModel
   DataMining
   DataAccess
   DistributedDataAccess
   GeoPhysics
   DerivedProperties
   Interpolation
   LinearAlgebra
   TableIO
   EosPack
   CBMGenerics
   Serial_Hdf5
   OneFilePerProcess
   Parallel_Hdf5
   utilities
   FileSystem
   Utilities_Petsc
   LinearAlgebra
   Interpolation
   CrustalThickness
)

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
   ${MPI_INCLUDE_DIRS}
   ${PETSC_INCLUDE_DIRS}
   ${FLEXLM_INCLUDE_DIRS}
)

target_link_libraries(${APP_NAME}
   DataModel
   DataMining
   DataAccess
   DistributedDataAccess
   GeoPhysics
   DerivedProperties
   Interpolation
   LinearAlgebra
   TableIO
   EosPack
   CBMGenerics
   Serial_Hdf5
   Parallel_Hdf5
   utilities
   FileSystem
   Utilities_Petsc
   LinearAlgebra
   Interpolation
   CrustalThickness
   ${HDF5_LIBRARIES}
   ${PETSC_LIBRARIES}
)


# OFPP only for Linux
if (UNIX)
   target_link_libraries(${APP_NAME} OneFilePerProcess)
   bm_include_libraries( OneFilePerProcess )
endif (UNIX)

# Add FLEXLM if enabled
if (BM_USE_FLEXLM)
   include_directories( ${FLEXLM_INCLUDE_DIRS})
   target_link_libraries(${APP_NAME} ${FLEXLM_LIBRARIES})
endif(BM_USE_FLEXLM)

set_target_properties(${APP_NAME}
   PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)

install(TARGETS ${APP_NAME}
   RUNTIME DESTINATION bin
)

# Create doxygen
generate_dox(fastctc.cfg)

endif(BM_PARALLEL)
# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:

