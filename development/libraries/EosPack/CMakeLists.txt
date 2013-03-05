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
