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

#FILE( GLOB all_headers src/*.h )
#FILE( GLOB all_srcs src/*.cpp)
set (all_headers,
   src/AbstractPropertiesCalculator.h
   src/HdfPropertiesCalculator.h
   src/VisualizationPropertiesCalculator.h
   src/Utilities.h
   src/ExportToHDF.h )

set(all_srcs
   src/AbstractPropertiesCalculator.cpp
   src/HdfPropertiesCalculator.cpp
   src/VisualizationPropertiesCalculator.cpp
   src/Utilities.cpp
   src/ExportToHDF.cpp
   src/main.cpp)

source_group(include FILES ${all_headers})

source_group(source FILES ${all_srcs})

set( APP_NAME "fastproperties" )
add_executable(${APP_NAME}
   ${all_srcs}
   ${all_headers}
)

set_target_properties( ${APP_NAME} PROPERTIES SUFFIX ".exe" )
set_target_properties( ${APP_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${APP_NAME}" )

create_application_run_script( ${APP_NAME} )

application_env_var( TARGET ${APP_NAME} VARS
     "GENEX5DIR"   "$MISCDIR/genex50"
     "GENEX6DIR"   "$MISCDIR/genex60"
     "OTGCDIR"     "$MISCDIR/OTGC"
     "EOSPACKDIR"  "$MISCDIR/eospack"
     "CTCDIR"      "$MISCDIR"
)

include_directories( SYSTEM
   ${PROJECT_SOURCE_DIR}/libraries/FastCauldron/src
   ${HDF5_INCLUDE_DIRS}
   ${MPI_INCLUDE_DIRS}
   ${PETSC_INCLUDE_DIRS}
   ${PUGIXML_INCLUDE_DIR}
   ${LZ_INCLUDE_DIR}
)

target_link_libraries(${APP_NAME}
   FastcauldronLib
   AbstractDerivedProperties
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

install(TARGETS ${APP_NAME}
   RUNTIME DESTINATION bin
)

if (UNIX)
   # In LSF environment (on LSF cluster node, when build is running as a LSF job) mpirun is trying to use job settings
   # to run mpi unit tests. Sometime it fails because build job requested just 1 cpu. To prevent this we can specify
   # machines file with localhost list only
   configure_file(test/machines machines COPYONLY )
   set( MACHINE_FILE -machinefile machines)
endif (UNIX)

add_gtest( NAME Fastproperties::ConvertProperties_MPInp2
           SOURCES test/ConvertProperties.cpp src/Utilities.cpp src/Utilities.h
           LIBRARIES DerivedProperties VisualizationIO DataAccess DistributedDataAccess  GeoPhysics  Parallel_Hdf5 VisualizationIO_projectHandle
           MPI_SIZE 2
           MPIRUN_PRMS ${MACHINE_FILE}
           FOLDER "${BASE_FOLDER}/${APP_NAME}"
        )

generate_dox( fastproperties.cfg )

