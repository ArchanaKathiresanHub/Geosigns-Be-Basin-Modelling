
if (NOT MSVC)
# Note: Source code in this module makes use of an Intel extension to C++ which is
# not supported by MS VC++. Therefore it won't compile without modification on
# MS VC++

add_library(calibration_kernel
	src/CalibrationNode.C
	src/SmectiteIlliteSimulator.C
	src/SmectiteIlliteSimulatorState.C
	src/LinearInterpolator.C
	src/BiomarkersSimulator.C
	src/ftanalysis.C
	src/ftgrain.C
	src/ftparameters.C
	src/ftsampleobservation.C
	src/ftsampleprediction.C
	src/histogram.C
)

bm_include_libraries( CBMGenerics )

install(TARGETS calibration_kernel
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

endif(NOT MSVC)

generate_dox( src/calibration_kernel.cfg )
