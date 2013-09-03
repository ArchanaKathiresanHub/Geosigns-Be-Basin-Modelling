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
   SOURCES test/PTDiagramBuilderTest.C test/PVTCfgFileMgr.C
   LIBRARIES EosPack
)

######################################
# Unit test for PTDiagramBuilderTest in multithreaded environment
add_gtest( NAME PTDiagramCalculator_MT
   SOURCES test/PTDiagramBuilderMTTest.C test/PVTCfgFileMgr.C
   LIBRARIES EosPack
   COMPILE_FLAGS "${OpenMP_CXX_FLAGS}"
   LINK_FLAGS "${OpenMP_CXX_FLAGS} -openmp-link static"
)

######################################
# Unit test for flasher
add_gtest( NAME pvtFlash::CAPI
   SOURCES test/EosPackTest.C test/PVTCfgFileMgr.C
   LIBRARIES EosPack
)

######################################
# Unit test for flasher in multithreaded environment
add_gtest( NAME pvtFlash::CAPI_MT
   SOURCES test/EosPackMTTest.C test/PVTCfgFileMgr.C
   LIBRARIES EosPack
   COMPILE_FLAGS "${OpenMP_CXX_FLAGS}"
   LINK_FLAGS "${OpenMP_CXX_FLAGS} -openmp-link static"
)

