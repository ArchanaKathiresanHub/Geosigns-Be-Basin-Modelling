#########################################################################
#                                                                       #
# Copyright (C) 2014 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

if (UNIX)
   set(preciseFpModel)
   if (BM_USE_INTEL_COMPILER)
      set(preciseFpModel "-fp-model precise")
   endif()
   set(cxxVersion "-std=c++11")
   MESSAGE(STATUS "C++ version is set to ${cxxVersion}")

   set(CMAKE_C_FLAGS_RELEASE "-g -O2" CACHE STRING "List of C compiler flags for a Release build") 
   set(CMAKE_CXX_FLAGS_RELEASE "-g -O2 ${cxxVersion}" CACHE STRING "List of C++ compiler flags for a Release build") 

   set(CMAKE_C_FLAGS_DEBUG"-g -O0 ${preciseFpModel}" CACHE STRING "List of C compiler flags for a Debug build") 
   set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 ${preciseFpModel} ${cxxVersion}" CACHE STRING "List of C++ compiler flags for a Debug build") 

   set(CMAKE_C_FLAGS_DEBUGALL "-g -O0 -DGLIBCXX_DEBUG ${preciseFpModel}" CACHE STRING "List of C compiler flags for a DebugAll build") 
   set(CMAKE_CXX_FLAGS_DEBUGALL "-g -O0 -DGLIBCXX_DEBUG ${preciseFpModel} ${cxxVersion}" CACHE STRING "List of C++ compiler flags for a DebugAll build") 
   set(CMAKE_C_FLAGS_MEMCHECK "-g -O0 ${preciseFpModel}" CACHE STRING "List of C compiler flags for a MemCheck build") 
   set(CMAKE_CXX_FLAGS_MEMCHECK "-g -O0 ${preciseFpModel} ${cxxVersion}" CACHE STRING "List of C++ compiler flags for a MemCheck build") 

   if (CMAKE_BUILD_TYPE STREQUAL MemCheck)
     if (NOT BUILD_SHARED_LIBS)
        message(WARNING "MemCheck build with statically linked MPI libraries will give lots of false positives")
     endif()
      if (BM_USE_INTEL_COMPILER)
        message(WARNING "MemCheck build with Intel compiler will give lots of false positives")
     endif()
  endif()

endif()

