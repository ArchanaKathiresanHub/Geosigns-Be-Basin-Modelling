#
# ATTENTION: The file is used in Basin Modeling and GEOCASE projects
#

add_library(CBMGenerics
	src/ComponentManager.C
	src/GenexResultManager.C
	src/capillarySealStrength.C
	src/fracturePressure.C
	src/waterDensity.C
	src/methaneSolubility.C
	src/Polyfunction.C
)

generate_dox( src/CBMGenerics.cfg )

install(TARGETS CBMGenerics
	RUNTIME DESTINATION bin
	LIBRARY	DESTINATION lib
	ARCHIVE DESTINATION lib
)

