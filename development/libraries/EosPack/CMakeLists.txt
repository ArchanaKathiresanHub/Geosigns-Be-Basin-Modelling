
set(EOSPACK_SOURCES
	src/EosPack.C
	src/EosCauldron.C
	src/EosPvtModel.C
	src/EosPvtTable.C
	src/EosUtils.C
	src/EosWater.C
	src/Parse.C
	src/polynomials.C
	src/PTDiagCalculator.C
)

set(EOSPACK_LIB_MODE)
if (WIN32 AND M_BUILD_EOSPACK_STATIC)
  set(EOSPACK_LIB_MODE STATIC)
elseif(WIN32)
  set(EOSPACK_LIB_MODE SHARED)
endif()

add_library(EosPack ${EOSPACK_LIB_MODE}
	${EOSPACK_SOURCES}
	${EOSPACK_EXTRA_SOURCES}
)

bm_include_libraries(CBMGenerics)

target_link_libraries(EosPack CBMGenerics)
set_target_properties(EosPack
  PROPERTIES DEFINE_SYMBOL EOSPACK_EXPORTS
)

generate_dox( src/EosPack.cfg )

install(TARGETS EosPack
	RUNTIME DESTINATION bin
	LIBRARY	DESTINATION lib
	ARCHIVE DESTINATION lib
)


# ###### Compile and run UnitTests on MS Windows
if (BM_CSHARP_API)
	configure_file(test/AssemblyInfo.cs.cmake csharp-test/Properties/AssemblyInfo.cs)
	configure_file(test/EosPackTest.cs csharp-test/EosPackTest.cs)
	add_csharp_unittest( 
	  NAME EosPack.Test
	  PLATFORM ${BM_WINDOWS_PLATFORM}
	  DIRECTORY csharp-test
	  DEPENDS EosPack
	  TESTLIST "Test"
	  TFS_SERVER_URL ${BM_TFS_SERVER_URL}
	  TFS_BUILD_NUMBER ${BM_TFS_BUILD_NUMBER}
	  TFS_PROJECT_NAME ${BM_TFS_PROJECT_NAME}
	  DEPLOYMENT_ITEMS "Debug/EosPack.dll" "Release/EosPack.dll"
	)
endif (BM_CSHARP_API)
	  
# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End: