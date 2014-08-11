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

set(cross_platform_headers
	  src/ChemicalCompactionCalculator.h
      src/Permeability.h
      src/PermeabilityImpermeable.h
      src/PermeabilityMudStone.h
      src/PermeabilityMultiPoint.h
      src/PermeabilitySandStone.h
      src/SchneiderCompactionCalculator.h
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
      src/TimeDependantLithology.h
   )

set(cross_platform_srcs
	  src/ChemicalCompactionCalculator.C
      src/Permeability.C
      src/PermeabilityMudStone.C
      src/PermeabilityMultiPoint.C
      src/PermeabilitySandStone.C
      src/SchneiderCompactionCalculator.C
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
      src/TimeDependantLithology.C
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
      DataAccess
      TableIO
      CBMGenerics
      EosPack
)


set(all_srcs)
set(all_headers)
set(all_dependent_libs)

if (UNIX)
   set(all_srcs ${cross_platform_srcs} ${unix_only_srcs})
   set(all_headers ${cross_platform_headers} ${unix_only_headers})
   set(all_dependent_libs ${cross_platform_libs} ${unix_only_libs} )
elseif( WIN32 )
   set(all_srcs ${cross_platform_srcs})
   set(all_headers ${cross_platform_headers})
   set(all_dependent_libs ${cross_platform_libs} )
endif()

source_group(include FILES ${all_headers})
source_group(source FILES ${all_srcs})


add_library( GeoPhysics
	${all_srcs}
	${all_headers}
)

bm_include_libraries( 
        ${all_dependent_libs}
)
include_directories(
	${HDF5_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/libraries/DataModel/src
)

target_link_libraries(GeoPhysics
      ${all_dependent_libs}
)
	
install(TARGETS GeoPhysics	
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)


### Unit Tests

add_gtest( NAME GeoPhysics::ChemicalCompactionCalculator
      SOURCES test/ChemicalCompactionCalculatorTest.C 
      LIBRARIES GeoPhysics SerialDataAccess
)

add_gtest(NAME GeoPhysics::PermeabilityImpermeable
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
