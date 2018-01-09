#########################################################################
#                                                                       #
# Copyright (C) 2012-2017 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

if (BM_PARALLEL AND TSLIB_ROOT)

   set( APP_NAME "fasttouch7" )
   add_executable(${APP_NAME}
       src/main.C
       src/misc.C
       src/FastTouch.C
       src/MasterTouch.C
       src/BurialHistory.C
       src/WriteBurial.C
       src/TouchstoneFiles.C
   )

   set_target_properties( ${APP_NAME} PROPERTIES SUFFIX ".exe" )
   set_target_properties( ${APP_NAME} PROPERTIES FOLDER "${BASE_FOLDER}/${APP_NAME}" )

   create_application_run_script( ${APP_NAME} )

   application_env_var( TARGET ${APP_NAME} VARS
     "touchstoneWrapper" "$DIR/touchstoneWrapper"
     "GEOCOSMBASEDIR" "$MISCDIR/geocosm"
     "GEOCOSMDIR" "$GEOCOSMBASEDIR/bin/x86_64_linux/Release"
     "MATLABMCR" "${MCR_ROOT}"
     "LD_LIBRARY_PATH" "$LD_LIBRARY_PATH:/usr/lib64:$MATLABMCR/bin/glnxa64"
     "LANG" "en_US.UTF-8"
     "MWDISABLE_MVM_API" "1"
     "LD_LIBRARY_PATH" "$LD_LIBRARY_PATH:$MATLABMCR/sys/os/glnxa64:$MATLABMCR/runtime/glnxa64:$MATLABMCR/sys/java/jre/glnxa64/jre/lib/amd64/native_threads"
     "LD_LIBRARY_PATH" "$LD_LIBRARY_PATH:$MATLABMCR/sys/java/jre/glnxa64/jre/lib/amd64/server"
     "LD_LIBRARY_PATH" "$LD_LIBRARY_PATH:$MATLABMCR/sys/java/jre/glnxa64/jre/lib/amd64:$GEOCOSMBASEDIR/bin/x86_64_linux/Release"
     "XAPPLRESDIR" "$MATLABMCR/X11/app-defaults"
   )


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

   target_link_libraries(${APP_NAME}
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
      target_link_libraries(${APP_NAME}
            OneFilePerProcess
      )
   endif(UNIX)

   set_target_properties(${APP_NAME}
         PROPERTIES LINK_FLAGS "${PETSC_LINK_FLAGS}"
   )

   if (BM_USE_FLEXLM)
      include_directories(${FLEXLM_INCLUDE_DIRS})
      target_link_libraries(${APP_NAME} ${FLEXLM_LIBRARIES})
   endif(BM_USE_FLEXLM)

   install(TARGETS ${APP_NAME}
       RUNTIME DESTINATION bin
   )

   add_definitions(-DMPICH_IGNORE_CXX_SEEK)
   if (WIN32)
     #Min and Max defined in BurialHistorySchema.hxx collides with the min max macro
     add_definitions(-DNOMINMAX)
   endif()

   
   #Touchstone Wrapper
   set( WRAPPER_NAME "touchstoneWrapper" )
   add_executable(${WRAPPER_NAME}
       src/TouchstoneWrapper.C 
       src/TsLibLoader.C
       src/LibLoaderLinux.C
       src/Touchstone.C
       src/ReadBurial.C
       src/TouchstoneFiles.C
   )
   
   add_dependencies(${WRAPPER_NAME} TSLIB)

   target_link_libraries(${WRAPPER_NAME}
       ${TSLIB_LIBRARIES}
       ${XERCES_LIBRARIES}
       utilities
       ${BM_DL_LIB}
   )

   install(TARGETS ${WRAPPER_NAME}
       RUNTIME DESTINATION bin
   )

   ### Unit Tests
   add_gtest(NAME WriteReadBurialHistory
      SOURCES test/BurialHistory.C src/WriteBurial.C src/ReadBurial.C
      INCLUDE_DIRS ${TSLIB_INCLUDE_DIRS} ${XERCES_INCLUDE_DIRS} ${XSD_INCLUDE_DIRS}
      DEPENDS TSLIB
      LIBRARIES ${TSLIB_LIBRARIES} ${XERCES_LIBRARIES}
      FOLDER "${BASE_FOLDER}/${APP_NAME}"
   )

   add_gtest(NAME WriteReadTouchstone
      SOURCES test/Touchstone.C src/TouchstoneFiles.C
      INCLUDE_DIRS ${TSLIB_INCLUDE_DIRS} ${XERCES_INCLUDE_DIRS} ${XSD_INCLUDE_DIRS}
      DEPENDS TSLIB
      LIBRARIES ${TSLIB_LIBRARIES} ${XERCES_LIBRARIES}
      FOLDER "${BASE_FOLDER}/${APP_NAME}"
   )

endif(BM_PARALLEL AND TSLIB_ROOT)

# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
