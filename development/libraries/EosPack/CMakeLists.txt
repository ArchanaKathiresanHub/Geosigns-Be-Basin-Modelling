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

add_gtest( NAME PTDiagramCalculator
   SOURCES test/PTDiagramBuilderTest.C
   LIBRARIES EosPack
)

######################################
# Unit test for flasher
add_gtest( NAME pvtFlash::CAPI
   SOURCES test/EosPackTest.C
   LIBRARIES EosPack
)


