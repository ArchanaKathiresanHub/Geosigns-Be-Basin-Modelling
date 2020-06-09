#########################################################################
#                                                                       #
# Copyright (C) 2012-2018 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

add_executable(deviatedwell
   src/DeviatedWellData.cpp
   src/deviatedwell.cpp
   src/GlobalDefs.cpp
   src/WellTrajectoryLocation.cpp
   src/CSVWellWriter.cpp
   src/MatlabWellWriter.cpp
   src/StdOutWellWriter.cpp
   src/StreamWellWriter.cpp
   src/WellWriter.cpp
   src/WellWriterFactory.cpp
)

include_directories( SYSTEM
   ${HDF5_INCLUDE_DIRS}
)

target_link_libraries(deviatedwell
   DataMining
   GeoPhysics
   AbstractDerivedProperties
   DerivedProperties
   DataAccess
   SerialDataAccess
   EosPack
   utilities
   Serial_Hdf5
   TableIO
   LinearAlgebra
   Interpolation
   FiniteElements
   CBMGenerics
   calibration_kernel
   ${HDF5_LIBRARIES}
)

install(TARGETS deviatedwell
   RUNTIME DESTINATION bin
)
