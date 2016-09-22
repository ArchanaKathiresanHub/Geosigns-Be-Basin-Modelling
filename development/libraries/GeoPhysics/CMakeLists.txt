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

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -axCORE-AVX2 -qopenmp -simd")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
# Vectorization is not enabled on Visual Studio
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

set(cross_platform_headers
      src/BrinePhases.h
      src/BrineConductivity.h
      src/BrineDensity.h
      src/BrineVelocity.h
      src/BrineViscosity.h
      src/ChemicalCompactionCalculator.h
      src/DoubleExponentialPorosity.h
      src/ExponentialPorosity.h
      src/Permeability.h
      src/Porosity.h
      src/PermeabilityImpermeable.h
      src/PermeabilityMudStone.h
      src/PermeabilityMultiPoint.h
      src/PermeabilitySandStone.h
      src/SeismicVelocity.h
      src/SeismicVelocityGardner.h
      src/SeismicVelocityKennan.h
      src/SeismicVelocityWyllie.h
      src/SchneiderCompactionCalculator.h
      src/SoilMechanicsPorosity.h
      src/VreTtiMethod.h
      src/VreArrheniusMethod.h
      src/VitriniteReflectance.h
      src/WalderhaugCompactionCalculator.h
)

set(unix_only_headers
      src/AllochthonousLithologyInterpolator.h
      src/AllochthonousLithologyManager.h
      src/BasementLithology.h
      src/CauldronGridDescription.h
      src/CompoundLithologyArray.h
      src/CompoundLithologyComposition.h
      src/CompoundLithology.h
      src/CompoundProperty.h
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
      src/LithologyManager.h
      src/Local2DArray.h
      src/Local3DArray.h
      src/SimpleLithology.h
      src/TimeDependentLithology.h
   )

set(cross_platform_srcs
      src/BrinePhases.C
      src/BrineConductivity.C
      src/BrineDensity.C
      src/BrineVelocity.C
      src/BrineViscosity.C
      src/ChemicalCompactionCalculator.C
      src/DoubleExponentialPorosity.C
      src/ExponentialPorosity.C
      src/Permeability.C
      src/Porosity.C
      src/PermeabilityMudStone.C
      src/PermeabilityMultiPoint.C
      src/PermeabilitySandStone.C
      src/SeismicVelocity.C
      src/SeismicVelocityGardner.C
      src/SeismicVelocityKennan.C
      src/SeismicVelocityWyllie.C
      src/SchneiderCompactionCalculator.C
      src/SoilMechanicsPorosity.C
      src/VreTtiMethod.C
      src/VreArrheniusMethod.C
      src/VitriniteReflectance.C
      src/WalderhaugCompactionCalculator.C
)

set(unix_only_srcs
      src/AllochthonousLithologyInterpolator.C
      src/AllochthonousLithologyManager.C
      src/BasementLithology.C
      src/CauldronGridDescription.C
      src/CompoundLithologyArray.C
      src/CompoundLithology.C
      src/CompoundLithologyComposition.C
      src/CompoundProperty.C
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
      src/LithologyManager.C
      src/SimpleLithology.C
      src/TimeDependentLithology.C
   )

set(unix_only_libs
      LinearAlgebra 
      Interpolation 
      genex6 
      genex6_kernel 
      CrustalThickness
)

set(cross_platform_libs
      utilities 
      FileSystem
      DataAccess
      TableIO
      CBMGenerics
      EosPack
)


set(all_srcs)
set(all_headers)
set(all_dependent_libs)

set(all_srcs ${cross_platform_srcs} ${unix_only_srcs})
set(all_headers ${cross_platform_headers} ${unix_only_headers})
set(all_dependent_libs ${cross_platform_libs} ${unix_only_libs} )

source_group(include FILES ${all_headers})
source_group(source FILES ${all_srcs})


add_library( GeoPhysics 
	${all_srcs}
	${all_headers}
)

# under Linux we are building Boost that is why this target exist
if (UNIX)
add_dependencies(GeoPhysics Boost)
endif (UNIX)

bm_include_libraries( 
        DataModel
        ${all_dependent_libs}
)
include_directories(
	${HDF5_INCLUDE_DIRS}
)

target_link_libraries(GeoPhysics
      ${all_dependent_libs}
)
	
install(TARGETS GeoPhysics	
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

generate_dox( GeoPhysics.cfg )


### Unit Tests

# BrineConductivity unit test not available since reading the FltThCondIoTbl from the project file
#add_gtest( NAME GeoPhysics::BrineConductivity
#      SOURCES test/BrineConductivity.C src/BrinePhases.C
#      LIBRARIES GeoPhysics SerialDataAccess
#)

add_gtest( NAME GeoPhysics::BrineDensity
      SOURCES test/BrineDensity.C src/BrinePhases.C
      LIBRARIES GeoPhysics SerialDataAccess
)

add_gtest( NAME GeoPhysics::BrineVelocity
      SOURCES test/BrineVelocity.C src/BrinePhases.C
      LIBRARIES GeoPhysics SerialDataAccess
)

add_gtest( NAME GeoPhysics::BrineViscosity
      SOURCES test/BrineViscosity.C src/BrinePhases.C
      LIBRARIES GeoPhysics SerialDataAccess
)

add_gtest( NAME GeoPhysics::ChemicalCompactionCalculator
      SOURCES test/ChemicalCompactionCalculatorTest.C 
      LIBRARIES GeoPhysics SerialDataAccess
)

add_gtest( NAME GeoPhysics::DoubleExponentialPorosity
      SOURCES test/DoubleExponentialPorosity.C 
      LIBRARIES GeoPhysics SerialDataAccess
)

add_gtest( NAME GeoPhysics::ExponentialPorosity
      SOURCES test/ExponentialPorosity.C 
      LIBRARIES GeoPhysics SerialDataAccess
)

add_gtest( NAME GeoPhysics::MixingPermeability
   SOURCES test/MixingPermeability.C
   LIBRARIES GeoPhysics DataAccess SerialDataAccess
)

add_gtest( NAME GeoPhysics::PermeabilityImpermeable
   SOURCES test/PermeabilityImpermeable.C
   LIBRARIES GeoPhysics SerialDataAccess
)

add_gtest( NAME GeoPhysics::PermeabilitySandStone
   SOURCES test/PermeabilitySandStone.C
   LIBRARIES GeoPhysics SerialDataAccess
)

add_gtest( NAME GeoPhysics::PermeabilityMudStone
      SOURCES test/PermeabilityMudStone.C
      LIBRARIES GeoPhysics SerialDataAccess
)

add_gtest( NAME GeoPhysics::PermeabilityMultiPoint
      SOURCES test/PermeabilityMultiPoint.C
      LIBRARIES GeoPhysics SerialDataAccess
)

add_gtest( NAME GeoPhysics::SchneiderCompactionCalculator
      SOURCES test/SchneiderCompactionCalculatorTest.C 
      LIBRARIES GeoPhysics SerialDataAccess
)

add_gtest( NAME GeoPhysics::SeismicVelocityGardner
      SOURCES test/SeismicVelocityGardner.C 
      LIBRARIES GeoPhysics SerialDataAccess
)

add_gtest( NAME GeoPhysics::SeismicVelocityKennan
      SOURCES test/SeismicVelocityKennan.C 
      LIBRARIES GeoPhysics SerialDataAccess
)

add_gtest( NAME GeoPhysics::SeismicVelocityWyllie
      SOURCES test/SeismicVelocityWyllie.C 
      LIBRARIES GeoPhysics SerialDataAccess
)

add_gtest( NAME GeoPhysics::SoilMechanicsPorosity
      SOURCES test/SoilMechanicsPorosity.C 
      LIBRARIES GeoPhysics SerialDataAccess
)

add_gtest( NAME GeoPhysics::VitriniteReflectance
      SOURCES test/VitriniteReflectance.C test/InputGrid.C test/OutputGrid.C
      LIBRARIES GeoPhysics SerialDataAccess
)

add_gtest( NAME GeoPhysics::VreTtiMethod
      SOURCES test/VreTtiMethod.C test/InputGrid.C test/OutputGrid.C
      LIBRARIES GeoPhysics SerialDataAccess
)

add_gtest( NAME GeoPhysics::VreArrheniusMethod
      SOURCES test/VreArrheniusMethod.C test/InputGrid.C test/OutputGrid.C
      LIBRARIES GeoPhysics SerialDataAccess
)

add_gtest( NAME GeoPhysics::WalderhaugCompactionCalculator
      SOURCES test/WalderhaugCompactionCalculatorTest.C 
      LIBRARIES GeoPhysics SerialDataAccess
)


# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
