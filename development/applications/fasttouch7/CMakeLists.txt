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

if (BM_PARALLEL AND TSLIB_ROOT)

set( FASTTOUCH7_TARGET_NAME "fasttouch7.exe" )
add_executable(${FASTTOUCH7_TARGET_NAME}
    src/main.C
    src/misc.C
    src/FastTouch.C
    src/MasterTouch.C
    src/BurialHistory.C
    src/WriteBurial.C
    src/TouchstoneFiles.C
)

create_application_run_script( ${FASTTOUCH7_TARGET_NAME} )

if ( "${BM_CONFIG_PRESET}" STREQUAL "ShellLinux" )
application_env_var( TARGET ${FASTTOUCH7_TARGET_NAME} VARS
  "LD_PRELOAD" "/glb/data/hpcrnd/easybuild/PRODUCTION/software/rhel/6/GCCcore/4.9.3/lib64/libstdc++.so"
  "MATLABMCR" "${MCR_ROOT}"
  "LD_LIBRARY_PATH" "$MATLABMCR/bin/glnxa64:$LD_LIBRARY_PATH"
  "LANG" "en_US.UTF-8"
  "MWDISABLE_MVM_API" "1"
  "LD_LIBRARY_PATH" "$MATLABMCR/sys/os/glnxa64:$MATLABMCR/runtime/glnxa64:$MATLABMCR/sys/java/jre/glnxa64/jre/lib/amd64/native_threads:$LD_LIBRARY_PATH"
  "LD_LIBRARY_PATH" "$MATLABMCR/sys/java/jre/glnxa64/jre/lib/amd64/server:$LD_LIBRARY_PATH"
  "LD_LIBRARY_PATH" "$MATLABMCR/sys/java/jre/glnxa64/jre/lib/amd64:$GEOCOSMBASEDIR/bin/x86_64_linux/Release:$LD_LIBRARY_PATH"
  "XAPPLRESDIR" "$MATLABMCR/X11/app-defaults"
  CUSTOM_COMMAND
  ". /glb/data/hpcrnd/easybuild/public/etc/profile.d/shell-envmodules.sh"
  "load module intel/2016.01"
)
else ()
application_env_var( TARGET ${FASTTOUCH7_TARGET_NAME} VARS
  "MATLABMCR" "${MCR_ROOT}"
  "LD_LIBRARY_PATH" "$MATLABMCR/bin/glnxa64:$LD_LIBRARY_PATH"
  "LANG" "en_US.UTF-8"
  "MWDISABLE_MVM_API" "1"
  "LD_LIBRARY_PATH" "$MATLABMCR/sys/os/glnxa64:$MATLABMCR/runtime/glnxa64:$MATLABMCR/sys/java/jre/glnxa64/jre/lib/amd64/native_threads:$LD_LIBRARY_PATH"
  "LD_LIBRARY_PATH" "$MATLABMCR/sys/java/jre/glnxa64/jre/lib/amd64/server:$LD_LIBRARY_PATH"
  "LD_LIBRARY_PATH" "$MATLABMCR/sys/java/jre/glnxa64/jre/lib/amd64:$GEOCOSMBASEDIR/bin/x86_64_linux/Release:$LD_LIBRARY_PATH"
  "XAPPLRESDIR" "$MATLABMCR/X11/app-defaults"
)

endif()


bm_include_libraries(
   DataModel
   DistributedDataAccess
   DataAccess
   Utilities_Petsc
   Serial_Hdf5
   OneFilePerProcess
   Parallel_Hdf5
   TableIO
   FileSystem
   utilities
   CBMGenerics
)

include_directories( SYSTEM
    ${HDF5_INCLUDE_DIRS}
    ${MPI_INCLUDE_DIRS}
    ${PETSC_INCLUDE_DIRS}
    ${XERCES_INCLUDE_DIRS}
    ${XSD_INCLUDE_DIRS}
    ${TSLIB_INCLUDE_DIRS}
)

target_link_libraries(${FASTTOUCH7_TARGET_NAME}
    DataAccess
    DistributedDataAccess
    Utilities_Petsc
    Serial_Hdf5
    Parallel_Hdf5
    TableIO
   FileSystem
    utilities
    CBMGenerics

    ${HDF5_LIBRARIES}
    ${PETSC_LIBRARIES}    
)

if (UNIX)
   target_link_libraries(${FASTTOUCH7_TARGET_NAME}
         OneFilePerProcess
   )
endif(UNIX)

set_target_properties(${FASTTOUCH7_TARGET_NAME}
      PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
)

if (BM_USE_FLEXLM)
   include_directories(${FLEXLM_INCLUDE_DIRS})
   target_link_libraries(${FASTTOUCH7_TARGET_NAME} ${FLEXLM_LIBRARIES})
endif(BM_USE_FLEXLM)

install(TARGETS ${FASTTOUCH7_TARGET_NAME}
    RUNTIME DESTINATION bin
)

add_definitions(-DMPICH_IGNORE_CXX_SEEK)
if (WIN32)
  #Min and Max defined in BurialHistorySchema.hxx collides with the min max macro
  add_definitions(-DNOMINMAX)
endif()

#Touchstone Wrapper

add_executable(touchstoneWrapper
    src/TouchstoneWrapper.C 
    src/TsLibLoader.C
    src/LibLoaderLinux.C
    src/Touchstone.C
    src/ReadBurial.C
    src/TouchstoneFiles.C
)

target_link_libraries(touchstoneWrapper
    ${TSLIB_LIBRARIES}
    ${XERCES_LIBRARIES}
    utilities
    ${BM_DL_LIB}
)

install(TARGETS touchstoneWrapper
    RUNTIME DESTINATION bin
)

### Unit Tests

add_gtest(NAME WriteReadBurialHistory
   SOURCES test/BurialHistory.C src/WriteBurial.C src/ReadBurial.C
   LIBRARIES ${TS7_LIBRARIES} ${TS_LIBRARIES} ${XERCES_LIBRARIES} 
)

add_gtest(NAME WriteReadTouchstone
   SOURCES test/Touchstone.C src/TouchstoneFiles.C
   LIBRARIES ${TS7_LIBRARIES} ${TS_LIBRARIES} ${XERCES_LIBRARIES} 
)

endif(BM_PARALLEL AND TSLIB_ROOT)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
