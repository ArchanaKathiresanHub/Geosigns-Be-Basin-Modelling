#
#
#

set(target_name ibslink)


file(GLOB SOURCE_FILES "${BB_ROOT}/${target_name}/src/*.[Ch]")

set(MODULE, ${target_name})

set (MODSRCD
  ibslink.def
  LNKmain.def
  LNKcfuncs.def
  LNKtopwin2d.def
  LNKuimultiopen.def
  LNKuiopenfile.def
  LNKuiextensions.def
  LNKuisavefile.def
  LNKuicalculator.def
  LNKuiproject.def
  LNKuigrid.def
  LNKuiproperty.def
  LNKuireconage.def
  LNKuistrattop.def
  LNKuixyzdefaults.def
)

find_program(TNG_PROGRAMM NAMES tng PATHS ${GEOCASE_BASE_DIR}/bin/Linux64g)
if(NOT TNG_PROGRAMM)
  message(FATAL_ERROR "tng program not found")
endif()
set(TNGPATH ${GEOCASE_BASE_DIR}/template)
set(TNG_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/tng.sh)
configure_file(tng.sh.cmake ${TNG_SCRIPT})
if(UNIX)
  execute_process(COMMAND chmod a+x ${TNG_SCRIPT})
endif()

set (MODSRCC rubygeneralIOPandB.C)
set (MODSRCH rubygeneralIOPandB.h)

set(MODSRCV)
set(MODSRCL)
set(MODSRCY)
set(MODSRCI) 
set(MODSRCO)

set(generated_files
  co.C
  co.h
  coP.h
  ibslink.C
  ibslinkcunfuncs.C
  ibslinkcunfuncs.h
  ibslinkcunfuncsP.h
  ibslink.h
  lnk.C
  lnk.h
  lnkP.h
  size.C
  size.h
  sizeP.h
  topwin2d.C
  topwin2d.h
  topwin2dP.h
  uicalculator.C
  uicalculator.h
  uicalculatorP.h
  uicollection.C
  uicollection.h
  uicollectionP.h
  uiextensions.C
  uiextensions.h
  uiextensionsP.h
  uimastergrid.C
  uimastergrid.h
  uimastergridP.h
  uimultifile.C
  uimultifile.h
  uimultifileP.h
  uimultiopen.C
  uimultiopen.h
  uimultiopenP.h
  uiopenfile.C
  uiopenfile.h
  uiopenfileP.h
  uiproject.C
  uiproject.h
  uiprojectP.h
  uiproperty.C
  uiproperty.h
  uipropertyitem.C
  uipropertyitem.h
  uipropertyitemP.h
  uipropertyP.h
  uireconage.C
  uireconage.h
  uireconageP.h
  uisavefile.C
  uisavefile.h
  uisavefileP.h
  uiselcoll.C
  uiselcoll.h
  uiselcollP.h
  uistrattop.C
  uistrattop.h
  uistrattopP.h
  uiworkgiocollection.C
  uiworkgiocollection.h
  uiworkgiocollectionP.h
  uixyzdefaults.C
  uixyzdefaults.h
  uixyzdefaultsP.h
  win.C
  win.h
  winP.h
)

set(generated_src_files)
foreach(file ${generated_files})
  set (generated_src_files ${generated_src_files} ${CMAKE_CURRENT_BINARY_DIR}/${file})
endforeach(file)

add_definitions(-DIBSFLEXLMVERSION="2008.04" -DSVNREVISION="-9999" -DIBSLONGVERSION="v2010.03" -DIBSSHORTVERSION="v2010.03")
add_definitions(-Dgocad_defined_bool)
if(Linux)
  message(STATUS "Linux is Linux.....")
endif()
add_definitions(-DLinux -DMODULENAME="ibslink" -DMODULEVER="1.0")

include_directories(/apps/3rdparty/EPTFlexLm/v9.2/include)
include_directories(${BB_ROOT}/RubyGeneralIO/src)
include_directories(${BB_ROOT}/RubyUtilities/src)
include_directories(${GEOCASE_BASE_DIR}/include/nogl)
include_directories(${GEOCASE_BASE_DIR}/include/gospel)
include_directories(${GEOCASE_BASE_DIR}/include/ui)
include_directories(${GEOCASE_BASE_DIR}/include/usys)
include_directories(${GEOCASE_BASE_DIR}/include/calc)
include_directories(${GEOCASE_BASE_DIR}/include/dds)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(GEOCASE_GOCAD_ENABLED)
  add_definitions(-DGOCAD_ENABLED)
  include_directories(${GOCAD_INCLUDE_DIRS})
  link_directories(${GOCAD_LINK_DIRS})
  set(GOCAD_LINK_LIBS 
    ${GOCAD_LINK_DIR}/gocadlm_core.o
    #${GOCAD_LINK_DIR}/gocadlm_gui.o
    Gocad_gapi
    Gocad_archive
    Gocad_ascii
    Gocad_appli
    Gocad_tgobj
    Gocad_volumes
    Gocad_surfaces
    Gocad_lines
    Gocad_geobase
    Gocad_math
    Gocad_utils
    qt
    )
endif()

if(GEOCASE_HDF5_ENABLED)
  add_definitions(-DHDF5_ENABLED)
endif()

link_directories(
  /nfs/rvl/users/ibs/geocase_64/prod/v2.4/lib/Linux64g/static
  /nfs/rvl/users/ibs/geocase_64/utilities/Linux64g/xpm/lib
  /nfs/rvl/users/ibs/geocase_64/utilities/Linux64g/Xint/lib
)

add_executable(${target_name} ${SOURCE_FILES} ${generated_src_files})
target_link_libraries(${target_name} 
  RubyGeneralIO RubyUtilities 
  /apps/3rdparty/EPTFlexLm/v9.2/LinuxRHEL64_x86_64_30WS/lib64/EPTFlexLm.o
  gospel
  nogl 
  ui usys Xpm calc dds INT INTCGM Xm Xt X11
  )

if(GEOCASE_GOCAD_ENABLED)
  target_link_libraries(${target_name}
    ${GOCAD_LINK_LIBS}
    )
endif()

if(GEOCASE_HDF5_ENABLED)
  target_link_libraries(${target_name}
    Serial_Hdf5
    ${HDF5_hdf5_LIBRARY}
    )
endif()

add_custom_command(OUTPUT ${generated_files}
  COMMAND pwd
  COMMAND echo ${TNG_SCRIPT}
  COMMAND ${TNG_SCRIPT} ${BB_ROOT}/${target_name}/def/ibslink.def -macro VERSION 2008.04
  )

install(TARGETS ${target_name}
        RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

# end of file