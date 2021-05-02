#########################################################################
#                                                                       #
# Copyright (C) 2012-2018 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

if (BM_PARALLEL)

FILE( GLOB all_headers src/*.h )

FILE( GLOB all_srcs src/*.cpp src/*.cpp)

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
   AbstractDerivedProperties
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

### Unit Tests
configure_file(test/data/Project.project3d ${CMAKE_CURRENT_BINARY_DIR}/Project.project3d COPYONLY)
configure_file(test/data/Inputs.HDF ${CMAKE_CURRENT_BINARY_DIR}/Inputs.HDF COPYONLY)
configure_file(test/data/Project_CauldronOutputDir/HydrostaticDecompaction_Results.HDF ${CMAKE_CURRENT_BINARY_DIR}/Project_CauldronOutputDir/HydrostaticDecompaction_Results.HDF COPYONLY)
configure_file(test/data/Project_CauldronOutputDir/Time_0.000000.h5 ${CMAKE_CURRENT_BINARY_DIR}/Project_CauldronOutputDir/Time_0.000000.h5 COPYONLY)
configure_file(test/data/Project_CauldronOutputDir/Time_5.000000.h5 ${CMAKE_CURRENT_BINARY_DIR}/Project_CauldronOutputDir/Time_5.000000.h5 COPYONLY)
configure_file(test/data/Project_CauldronOutputDir/Time_13.000000.h5 ${CMAKE_CURRENT_BINARY_DIR}/Project_CauldronOutputDir/Time_13.000000.h5 COPYONLY)
configure_file(test/data/Project_CauldronOutputDir/Time_54.000000.h5 ${CMAKE_CURRENT_BINARY_DIR}/Project_CauldronOutputDir/Time_54.000000.h5 COPYONLY)
configure_file(test/data/Project_CauldronOutputDir/Time_65.000000.h5 ${CMAKE_CURRENT_BINARY_DIR}/Project_CauldronOutputDir/Time_65.000000.h5 COPYONLY)
configure_file(test/data/Project_CauldronOutputDir/Time_90.000000.h5 ${CMAKE_CURRENT_BINARY_DIR}/Project_CauldronOutputDir/Time_90.000000.h5 COPYONLY)
configure_file(test/data/Project_CauldronOutputDir/Time_142.000000.h5 ${CMAKE_CURRENT_BINARY_DIR}/Project_CauldronOutputDir/Time_142.000000.h5 COPYONLY)
configure_file(test/data/Project_CauldronOutputDir/Time_146.000000.h5 ${CMAKE_CURRENT_BINARY_DIR}/Project_CauldronOutputDir/Time_146.000000.h5 COPYONLY)
configure_file(test/data/Project_CauldronOutputDir/Time_155.000000.h5 ${CMAKE_CURRENT_BINARY_DIR}/Project_CauldronOutputDir/Time_155.000000.h5 COPYONLY)
configure_file(test/data/Project_CauldronOutputDir/Time_300.000000.h5 ${CMAKE_CURRENT_BINARY_DIR}/Project_CauldronOutputDir/Time_300.000000.h5 COPYONLY)
configure_file(test/data/Project_CauldronOutputDir/Output.iotables3d ${CMAKE_CURRENT_BINARY_DIR}/Project_CauldronOutputDir/Output.iotables3d COPYONLY)


add_gtest( NAME "Fasctc::CrustalThicknessCalculator"
    SOURCES test/CrustalThicknessCalculatorTest.cpp src/CrustalThicknessCalculator.cpp src/CrustalThicknessCalculatorFactory.cpp
    LIBRARIES   DataModel
                Utilities_Petsc
                DataMining
                DataAccess
                DistributedDataAccess
                GeoPhysics 
                CrustalThickness

    ENV_VARS CTCDIR=${PROJECT_SOURCE_DIR}/geocase/misc
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data
    FOLDER "${BASE_FOLDER}/${APP_NAME}"
)

# Create doxygen
generate_dox(fastctc.cfg)

endif(BM_PARALLEL)
