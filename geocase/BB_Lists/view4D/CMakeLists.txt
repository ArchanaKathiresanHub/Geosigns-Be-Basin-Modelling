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
include_directories(/nfs/rvl/groups/sgs/spirit2/APPL/OpenInventor/OpenInventor_6.1.2/Linux64_24_gcc3.4.6/include)
include_directories(${BB_ROOT}/../../libraries/TableIO/src)
include_directories(${BB_ROOT}/../../libraries/EosPack/src)

include_directories(/apps/3rdparty/EPTFlexLm/v9.2/include)

link_directories(
  /nfs/rvl/groups/sgs/spirit2/APPL/OpenInventor/OpenInventor_6.1.2/Linux64_24_gcc3.4.6/lib
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

# --- Linking optimized executable...
#/usr/bin/g++34 -m64 -Wno-deprecated -m64   view4D.o     GUI.o MenuBarCB.o MenuBarCreation.o allocObjects.o fileIO.o graph2D.o pdfViewCalc.o postUtils.o projectIO.o startfile.o trapPieChart.o writer.o traptracking.o  
#../../lib/Linux64/libRBYgeneralio.a 
#../../lib/Linux64/libRBYutilities.a 
#../../lib/Linux64/libdatabase.a 
#../../lib/Linux64/libserialhdf5.a 
#../../lib/Linux64/libEosPack.a 
#../../lib/Linux64/libCBMGenerics.a 
#../../lib/Linux64/libdatabase.a 
#/nfs/rvl/groups/ept-sg/SWEast/Cauldron/hpc/hdf5/1.6.6/LinuxRHEL64_x86_64_57WS/lib/libhdf5.a 
#-L/nfs/rvl/groups/sgs/spirit2/APPL/OpenInventor/OpenInventor_6.1.2/Linux64_24_gcc3.4.6/lib 
#-lInventorXt -lInventor -lMeshVizXt -lMeshViz -lScaleViz -lHardCopyXt -lHardCopy -limage 
#-L/glb/home/ksaho3/X11R6/lib64 -lXm -lXt -lXi -lXext -lm -lpthread -lGLU -lGL -lX11 -lz   
#-L/nfs/rvl/users/ibs/geocase_64/prod/v2.4/lib/Linux64g/static -lgospel -ldds -ldds -lc -lm  -o view4D

install(TARGETS ${target_name}
        RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)
# end of file
