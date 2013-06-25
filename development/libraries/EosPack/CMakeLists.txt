FILE( GLOB all_headers src/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.C)
source_group(source FILES ${all_srcs})


add_library(EosPack ${EOSPACK_LIB_MODE}
   ${all_srcs}
   ${all_headers}
)

bm_include_libraries(CBMGenerics)

target_link_libraries(EosPack CBMGenerics) 

generate_dox( src/EosPack.cfg )


#######################################
### Unit Tests
#######################################

#######################################
# Unit tests for PTDiagramCalculator

add_executable( PTDiagramBuilderTest
   test/PTDiagramBuilderTest.C
)

target_link_libraries( PTDiagramBuilderTest
   EosPack
)

# Unit test for C like API
add_test( NAME pvtFlash::BuildPTDiagram
   COMMAND PTDiagramBuilderTest capi
)

# Unit test for search Bubble/Dew lines
add_test( NAME PTDiagramCalculator::findBubbleDewLines
   COMMAND PTDiagramBuilderTest bbdw
)

# Unit test for countour line calculation
add_test( NAME PTDiagramCalculator::calcContourLine
   COMMAND PTDiagramBuilderTest isol
)

# Unit test for set of countour line calculation
add_test( NAME PTDiagramCalculator::calcContourLines
   COMMAND PTDiagramBuilderTest ain1
)

######################################
# Unit test for flasher
add_executable( EosPackTest
   test/EosPackTest.C
)

target_link_libraries( EosPackTest
   EosPack
)

# Unit test for flasher
add_test( NAME pvtFlash::EosPackComputeWithLumping
   COMMAND EosPackTest cwlmp_capi
)

# Unit test for flasher
add_test( NAME pvtFlash::Gorm
   COMMAND EosPackTest gorm_capi
)

# Unit test for flasher
add_test( NAME pvtFlash::GetMolWeight
   COMMAND EosPackTest gmw_capi
)

# Unit test for flasher
#add_test( NAME EosPack::computeWithLumping
#   COMMAND EosPackTest cwlmp
#)



