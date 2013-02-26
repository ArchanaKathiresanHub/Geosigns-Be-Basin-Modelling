FILE( GLOB all_headers src/*.h )
source_group(include FILES ${all_headers})

FILE( GLOB all_srcs src/*.C)
source_group(source FILES ${all_srcs})


set(EOSPACK_LIB_MODE)
if (WIN32 AND BM_BUILD_EOSPACK_STATIC)
  set(EOSPACK_LIB_MODE STATIC)
elseif(WIN32)
  set(EOSPACK_LIB_MODE SHARED)
endif()

add_library(EosPack ${EOSPACK_LIB_MODE}
   ${all_srcs}
   ${all_headers}
)

bm_include_libraries(CBMGenerics)

target_link_libraries(CBMGenerics) 

generate_dox( src/EosPack.cfg )
