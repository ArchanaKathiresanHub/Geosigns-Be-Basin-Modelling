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


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -axCORE-AVX2 -qopenmp -simd")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
# Vectorization is not enabled on Visual Studio so we don't add any compiler flag
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

set( LIB_NAME "GeoPhysics" )

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

create_bm_library( TARGET ${LIB_NAME}
                   LIBRARIES genex6_kernel
                             DataAccess
                             Interpolation
                             LinearAlgebra
                             genex6
                             genex6_kernel
                             DataAccess
                             TableIO
                             EosPack
                             CBMGenerics
                             FileSystem
                             utilities
                   INSTALLTARGET )

generate_dox( GeoPhysics.cfg )


### Unit Tests

# BrineConductivity unit test not available since reading the FltThCondIoTbl from the project file
add_gtest( NAME ${LIB_NAME}::BrineConductivity
      SOURCES test/BrineConductivity.C src/BrinePhases.C
      LIBRARIES ${LIB_NAME} DataAccess SerialDataAccess
      ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50 CTCDIR=${CFGFLS}/
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::BrineDensity
      SOURCES test/BrineDensity.C src/BrinePhases.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::BrineVelocity
      SOURCES test/BrineVelocity.C src/BrinePhases.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::BrineViscosity
      SOURCES test/BrineViscosity.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::BrooksCorey
      SOURCES test/PceTest.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::ChemicalCompactionCalculator
      SOURCES test/ChemicalCompactionCalculatorTest.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::DoubleExponentialPorosity
      SOURCES test/DoubleExponentialPorosity.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::ExponentialPorosity
      SOURCES test/ExponentialPorosity.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::LambdaMixer
      SOURCES test/LambdaMixer.cpp
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

copy_test_file(MixingPermeabilityProject.project3d)

copy_test_file(ConductivityProject.project3d)

add_gtest( NAME ${LIB_NAME}::MixingPermeability
   SOURCES test/MixingPermeability.C
   LIBRARIES ${LIB_NAME} DataAccess SerialDataAccess
   ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50 CTCDIR=${CFGFLS}/
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::MultiMixingPermeability
   SOURCES test/MultiMixingPermeability.C
   LIBRARIES ${LIB_NAME} DataAccess SerialDataAccess
   ENV_VARS EOSPACKDIR=${CFGFLS}/eospack GENEX5DIR=${CFGFLS}/genex50 CTCDIR=${CFGFLS}/
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)


add_gtest( NAME ${LIB_NAME}::MixingPermeabilityArray3litho
   SOURCES test/MixingPermeabilityArray3Litho.C
   LIBRARIES ${LIB_NAME} SerialDataAccess
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::MixingPermeabilityArray2litho
   SOURCES test/MixingPermeabilityArray2Litho.C
   LIBRARIES ${LIB_NAME} SerialDataAccess
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::MixingPermeabilityArray1litho
   SOURCES test/MixingPermeabilityArray1Litho.C
   LIBRARIES ${LIB_NAME} SerialDataAccess
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)


add_gtest( NAME ${LIB_NAME}::PermeabilityImpermeable
   SOURCES test/PermeabilityImpermeable.C
   LIBRARIES ${LIB_NAME} SerialDataAccess
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::PermeabilitySandStone
   SOURCES test/PermeabilitySandStone.C
   LIBRARIES ${LIB_NAME} SerialDataAccess
   FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::PermeabilityMudStone
      SOURCES test/PermeabilityMudStone.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::PermeabilityMultiPoint
      SOURCES test/PermeabilityMultiPoint.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::PermeabilityMultiPointArrayTest
      SOURCES test/PermeabilityMultiPointArrayTest.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::SchneiderCompactionCalculator
      SOURCES test/SchneiderCompactionCalculatorTest.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::SeismicVelocityGardner
      SOURCES test/SeismicVelocityGardner.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::SeismicVelocityKennan
      SOURCES test/SeismicVelocityKennan.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::SeismicVelocityWyllie
      SOURCES test/SeismicVelocityWyllie.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::SoilMechanicsPorosity
      SOURCES test/SoilMechanicsPorosity.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::VitriniteReflectance
      SOURCES test/VitriniteReflectance.C test/InputGrid.C test/OutputGrid.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::VreTtiMethod
      SOURCES test/VreTtiMethod.C test/InputGrid.C test/OutputGrid.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::VreArrheniusMethod
      SOURCES test/VreArrheniusMethod.C test/InputGrid.C test/OutputGrid.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::WalderhaugCompactionCalculator
      SOURCES test/WalderhaugCompactionCalculatorTest.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::MultiCompoundPropertyTest
      SOURCES test/MultiCompoundPropertyTest.C
      LIBRARIES ${LIB_NAME} SerialDataAccess utilities
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)

add_gtest( NAME ${LIB_NAME}::ConfigFileAlc
      SOURCES test/ConfigFileAlc.C
      LIBRARIES ${LIB_NAME} SerialDataAccess
      ENV_VARS CTCDIR=${PROJECT_SOURCE_DIR}/geocase/misc
      FOLDER "${BASE_FOLDER}/${LIB_NAME}"
)
