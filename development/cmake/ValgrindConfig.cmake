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

if (UNIX AND CMAKE_BUILD_TYPE STREQUAL "MemCheck")

message(STATUS "Enabling Valgrind Memory Checker tests")

#
# Add Valgrind as external component that can always be rebuilt.
#
add_external_project_to_repository(
      NAME Valgrind
      VERSION 3.10.0
      ARCHIVE "${THIRD_PARTY_DIR}/sources/valgrind-3.10.0.tar.bz2"
      ARCHIVE_MD5 "7c311a72a20388aceced1aa5573ce970"
      CONFIGURE_COMMAND 
         "./configure" 
            "--prefix={ROOT}" 
      BUILD_COMMAND   "make"
      INSTALL_COMMAND "make" "install"
      CONFIGURE_OPTIONS 
        COMPILER "{CurrentCompiler}"   # Always use system default compiler
        MPI      "{CurrentMPI}"       "--with-mpicc={CC}"  # This enables building of Valgrind's MPI wrapper 
        SPEED    "MemCheck"           # Only enable when MemCheck configuration is active
        OS       "{CurrentPlatform}"     
        LINK     "Dynamic"            # Only enable when building with shared libraries
)

# 
# Add an executable wrapper script to run Valgrind memory checks
#
set(VALGRIND "${Valgrind_ROOT}/bin/valgrind")
set(VALGRIND_MPI "${Valgrind_ROOT}/lib/valgrind/libmpiwrap-amd64-linux.so")

set(VALGRIND_MEMCHECK "${CMAKE_BINARY_DIR}/valgrind_memcheck.sh")
set(VALGRIND_MEMCHECK_AUX "${CMAKE_BINARY_DIR}/aux/valgrind_memcheck.sh")

if (BM_USE_INTEL_MPI)
#  valgrind built with Intel MPI depends on libimf.so library which is located in Intel compiler folder
#  for this we need to setup path to this library
   set(SOURCE_INTEL_VARS "source ${INTEL_CXX_ROOT}/bin/compilervars.sh intel64")
else (BM_USE_INTEL_MPI)
   set(SOURCE_INTEL_VARS "")
endif (BM_USE_INTEL_MPI)

file(WRITE ${VALGRIND_MEMCHECK_AUX}
"#/!bin/bash
# Wrapper for memory check with Valgrind
# This file has been generated automatically - Do not edit!

${SOURCE_INTEL_VARS}
source ${CMAKE_BINARY_DIR}/envsetup.sh

export LD_PRELOAD=${VALGRIND_MPI}
${VALGRIND} --max-stackframe=8388656 --error-exitcode=1 --run-libc-freeres=no \"$@\" 
exit_status=$?

exit $exit_status
"
   )

# Make the file executable
file(COPY "${VALGRIND_MEMCHECK_AUX}"
     DESTINATION "${CMAKE_BINARY_DIR}"
     FILE_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_WRITE GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
     )

#
# Add a macro that allows easy addition of a memory check for an executable
#
macro(add_memcheck)

   set(oneValueArgs NAME EXECUTABLE)
   set(multiValueArgs PARAMETERS )
   cmake_parse_arguments("addMemcheck" 
                      "" 
                      "${oneValueArgs}"
                      "${multiValueArgs}"
                      ${ARGN}
         )

   add_test( NAME "MEMORY-CHECK-${addMemcheck_NAME}" 
         COMMAND ${VALGRIND_MEMCHECK} $<TARGET_FILE:${addMemcheck_EXECUTABLE}> ${addMemcheck_PARAMETERS}
   )

   add_dependencies( ${addMemCheck_NAME} Valgrind ${addMemcheck_EXECUTABLE})
endmacro()

else()

macro(add_memcheck)
# Do nothing!
endmacro()

endif()
