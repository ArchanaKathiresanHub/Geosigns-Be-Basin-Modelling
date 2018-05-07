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

set(all_headers
      src/AllochthonousLithologyInterpolator.h
      src/AllochthonousLithologyManager.h
      src/BasementLithology.h
      src/BrinePhases.h
      src/BrineConductivity.h
      src/BrineDensity.h
      src/BrineVelocity.h
      src/BrineViscosity.h
      src/BrooksCorey.h
      src/CauldronGridDescription.h
      src/ChemicalCompactionCalculator.h
      src/CompoundLithologyArray.h
      src/CompoundLithologyComposition.h
      src/CompoundLithology.h
      src/CompoundProperty.h
      src/ConfigFileParameter.h
      src/ConfigFileParameterAlc.h
      src/DoubleExponentialPorosity.h
      src/EffectiveCrustalThicknessCalculator.h
      src/ExponentialPorosity.h
      src/FracturePressureCalculator.h
      src/GeoPhysicalConstants.h
      src/GeoPhysicalFunctions.h
      src/GeoPhysicsCrustFormation.h
      src/GeoPhysicsFluidType.h
      src/GeoPhysicsFormation.h
      src/GeoPhysicsMantleFormation.h
      src/GeoPhysicsObjectFactory.h
      src/GeoPhysicsProjectHandle.h
      src/GeoPhysicsSourceRock.h
      src/HCDensity.h
      src/LambdaMixer.h
      src/LithologyManager.h
      src/MultiCompoundProperty.h
      src/Permeability.h
      src/PermeabilityImpermeable.h
      src/PermeabilityMixer.h
      src/PermeabilityMudStone.h
      src/PermeabilityMultiPoint.h
      src/PermeabilitySandStone.h
      src/Porosity.h
      src/SchneiderCompactionCalculator.h
      src/SeismicVelocity.h
      src/SeismicVelocityGardner.h
      src/SeismicVelocityKennan.h
      src/SeismicVelocityWyllie.h
      src/SimpleLithology.h
      src/SoilMechanicsPorosity.h
      src/TimeDependentLithology.h
      src/VreTtiMethod.h
      src/VreArrheniusMethod.h
      src/VitriniteReflectance.h
      src/WalderhaugCompactionCalculator.h
)

set(all_srcs
      src/AllochthonousLithologyInterpolator.C
      src/AllochthonousLithologyManager.C
      src/BasementLithology.C
      src/BrinePhases.C
      src/BrineConductivity.C
      src/BrineDensity.C
      src/BrineVelocity.C
      src/BrineViscosity.C
      src/BrooksCorey.cpp
      src/CauldronGridDescription.C
      src/ChemicalCompactionCalculator.C
      src/CompoundLithologyArray.C
      src/CompoundLithology.C
      src/CompoundLithologyComposition.C
      src/CompoundProperty.C
      src/ConfigFileParameterAlc.C
      src/DoubleExponentialPorosity.C
      src/EffectiveCrustalThicknessCalculator.C
      src/ExponentialPorosity.C
      src/FracturePressureCalculator.C
      src/GeoPhysicalFunctions.C
      src/GeoPhysicsCrustFormation.C
      src/GeoPhysicsFluidType.C
      src/GeoPhysicsFormation.C
      src/GeoPhysicsMantleFormation.C
      src/GeoPhysicsObjectFactory.C
      src/GeoPhysicsProjectHandle.C
      src/GeoPhysicsSourceRock.C
      src/HCDensity.C
      src/LambdaMixer.cpp
      src/LithologyManager.C
      src/MultiCompoundProperty.C
      src/Permeability.C
      src/PermeabilityMudStone.C
      src/PermeabilityMultiPoint.C
      src/PermeabilitySandStone.C
      src/Porosity.C
      src/PermeabilityMixer.C
      src/SchneiderCompactionCalculator.C
      src/SeismicVelocity.C
      src/SeismicVelocityGardner.C
      src/SeismicVelocityKennan.C
      src/SeismicVelocityWyllie.C
      src/SimpleLithology.C
      src/SoilMechanicsPorosity.C
      src/TimeDependentLithology.C
      src/VreTtiMethod.C
      src/VreArrheniusMethod.C
      src/VitriniteReflectance.C
      src/WalderhaugCompactionCalculator.C
)

set(unix_only_libs
      LinearAlgebra
      Interpolation
      genex6
      genex6_kernel
)

set(cross_platform_libs
      utilities
      FileSystem
      DataAccess
      TableIO
      CBMGenerics
      EosPack
)

set(all_dependent_libs)
set(all_dependent_libs ${cross_platform_libs} ${unix_only_libs})

source_group(include FILES ${all_headers})
source_group(source FILES ${all_srcs})

set( LIB_NAME "GeoPhysics" )
add_library( ${LIB_NAME}
   ${all_srcs}
   ${all_headers}
)
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${LIB_NAME}" )

# under Linux we are building Boost that is why this target exist
if (UNIX)
add_dependencies(${LIB_NAME} Boost)
endif (UNIX)

bm_include_libraries(
        DataModel
        ${all_dependent_libs}
)
include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

target_link_libraries(${LIB_NAME}
      ${all_dependent_libs}
)

install(TARGETS ${LIB_NAME}
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)

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

configure_file(
   ${CMAKE_CURRENT_SOURCE_DIR}/test/MixingPermeabilityProject.project3d
   MixingPermeabilityProject.project3d
   COPYONLY
)

configure_file(
   ${CMAKE_CURRENT_SOURCE_DIR}/test/MixingPermeabilityProject.project3d
   ConductivityProject.project3d
   COPYONLY
)

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


# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
