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

#
#
#

set(target_name view4D)

set(SOURCE_FILES 
  ${BB_ROOT}/${target_name}/src/GUI.C
  ${BB_ROOT}/${target_name}/src/MenuBarCB.C
  ${BB_ROOT}/${target_name}/src/MenuBarCreation.C
  ${BB_ROOT}/${target_name}/src/allocObjects.C
  ${BB_ROOT}/${target_name}/src/fileIO.C
  ${BB_ROOT}/${target_name}/src/graph2D.C
  ${BB_ROOT}/${target_name}/src/pdfViewCalc.C
  ${BB_ROOT}/${target_name}/src/postUtils.C
  ${BB_ROOT}/${target_name}/src/projectIO.C
  ${BB_ROOT}/${target_name}/src/startfile.C
  ${BB_ROOT}/${target_name}/src/trapPieChart.C
  ${BB_ROOT}/${target_name}/src/writer.C
  ${BB_ROOT}/${target_name}/src/traptracking.C
  ${BB_ROOT}/${target_name}/src/view4D.C
)

file(GLOB HEADER_FILES ${BB_ROOT}/${target_name}/src/*.h)

add_definitions(-DMODULENAME="${target_name}" -DMODULEVER="1.0")
add_definitions(-Dgocad_defined_bool)

include_directories(${BB_ROOT}/RubyGeneralIO/src)
include_directories(${BB_ROOT}/RubyUtilities/src)
include_directories(${BB_ROOT}/CBMGenerics/src)
include_directories(${GEOCASE_BASE_DIR}/include/gospel)
include_directories(${GEOCASE_BASE_DIR}/include/dds)
include_directories(${GEOCASE_BASE_DIR}/include/usys)
include_directories(/nfs/rvl/users/ibs/d/EPT-HM/BB/cauldron/src)
include_directories(${OPENINVENTOR_BASE_DIR}/include)
include_directories(${BB_ROOT}/../../libraries/TableIO/src)
include_directories(${PROJECT_BINARY_DIR}/TableIO)
include_directories(${BB_ROOT}/../../libraries/EosPack/src)

include_directories(/apps/3rdparty/EPTFlexLm/v9.2/include)

link_directories(
  ${OPENINVENTOR_BASE_DIR}/lib
  /glb/home/ksaho3/X11R6/lib64
  /nfs/rvl/users/ibs/geocase_64/prod/v2.4/lib/Linux64g/static
  )

add_executable(${target_name} ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(${target_name}
  RubyGeneralIO
  RubyUtilities
  TableIO
  EosPack
  CBMGenerics
  Serial_Hdf5
  InventorXt Inventor MeshVizXt MeshViz ScaleViz HardCopyXt HardCopy image
  Xm Xt Xi Xext pthread GLU GL X11
  gospel dds
)

install(TARGETS ${target_name}
        RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)
# end of file
