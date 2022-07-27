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

################################################################
######################### SET TARGETS ##########################
################################################################

#
# fastcauldron application
#
set( FASTCAULDRON_TARGET_NAME "fastcauldron" )
add_executable( ${FASTCAULDRON_TARGET_NAME} src/main.cpp )
set_target_properties( ${FASTCAULDRON_TARGET_NAME} PROPERTIES SUFFIX ".exe" )
set_target_properties( ${FASTCAULDRON_TARGET_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${FASTCAULDRON_TARGET_NAME}" )
create_application_run_script( ${FASTCAULDRON_TARGET_NAME} )

application_env_var( TARGET ${FASTCAULDRON_TARGET_NAME} VARS
     "GENEXDIR"    "$MISCDIR/genex40"
     "GENEX5DIR"   "$MISCDIR/genex50"
     "GENEX6DIR"   "$MISCDIR/genex60"
     "OTGCDIR"     "$MISCDIR/OTGC"
     "CTCDIR"      "$MISCDIR"
     "EOSPACKDIR"  "$MISCDIR/eospack"
)

#
# linearSolver application
#
set( LINEARSOLVER_TARGET_NAME "linearSolver" )
add_executable( ${LINEARSOLVER_TARGET_NAME} linearSolver/main.cpp )
set_target_properties( ${LINEARSOLVER_TARGET_NAME} PROPERTIES SUFFIX ".exe" )
set_target_properties( ${LINEARSOLVER_TARGET_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LINEARSOLVER_TARGET_NAME}" )
create_application_run_script( ${LINEARSOLVER_TARGET_NAME} )


################################################################
################### SET DEPENDENCIES/LINKS #####################
################################################################

if (BM_USE_FLEXLM)
   include_directories( ${FLEXLM_INCLUDE_DIRS})
endif(BM_USE_FLEXLM)

include_directories(
   ${PROJECT_SOURCE_DIR}/libraries/FastCauldron/src
)

include_directories( SYSTEM
    ${HDF5_INCLUDE_DIRS}
    ${MPI_INCLUDE_DIRS}
    ${PETSC_INCLUDE_DIRS}
    ${EIGEN_INCLUDE_DIRS}
)

#
# fastcauldron links to FastcauldronApplication only
# fastcauldron needs to be linked to DataAccess and DistributedDataAccess
# libraries even if they are already linked to FastcauldronApplication because of
# DataAccess and DistributedDataAccess crossdependencies
#
target_link_libraries(${FASTCAULDRON_TARGET_NAME}
    FastcauldronLib
    DataAccess
    DistributedDataAccess
)
target_link_libraries(${LINEARSOLVER_TARGET_NAME}
    FastcauldronLib
    DataAccess
    DistributedDataAccess
)

set_target_properties(${FASTCAULDRON_TARGET_NAME}
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)
set_target_properties(${LINEARSOLVER_TARGET_NAME}
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)

if (WIN32)
  # Windows.h defines 'min' and 'max' macros if NOMINMAX is not defined. These
  # macros collide with std::min and std::max
  add_definitions(-DNOMINMAX)
endif()

install(TARGETS ${FASTCAULDRON_TARGET_NAME} ${LINEARSOLVER_TARGET_NAME}
        RUNTIME DESTINATION bin
)

if(UNIX AND BM_BUILD_HYPRE)
   # Copying Hypre shared library in the fastcauldron build directory
   add_custom_command(TARGET ${FASTCAULDRON_TARGET_NAME} POST_BUILD
                      COMMAND ${CMAKE_COMMAND} -E copy ${HYPRE_LIBRARIES} ${CMAKE_CURRENT_BINARY_DIR} )
endif()


################################################################
######################## MEMORY CHECK ##########################
################################################################

configure_file(
   ${CMAKE_CURRENT_SOURCE_DIR}/test/MemCheck.project3d
   MemCheck.project3d
   COPYONLY
)

if(BM_BUILD_ENABLE_TESTING)
add_memcheck(
   NAME "Fastcauldron_SimpleItcoupled"
   EXECUTABLE ${FASTCAULDRON_TARGET_NAME}
   PARAMETERS "-project" "MemCheck.project3d" "-itcoupled"
)
endif(BM_BUILD_ENABLE_TESTING)

endif(BM_PARALLEL)

################################################################
########################### DOXYGEN ############################
################################################################

#generate_dox(src/fastcauldron.cfg)
