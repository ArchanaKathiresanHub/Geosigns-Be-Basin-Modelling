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

cmake_minimum_required(VERSION 2.8)
project(gpu-vs-cpu-benchmarks)

macro( target_enable_openmp TARGET )
  set(cflags)
  get_target_property(cflags ${TARGET} COMPILE_FLAGS)
  if (cflags)
    set_target_properties( ${TARGET}
	PROPERTIES
	COMPILE_FLAGS "${cflags};${OpenMP_C_FLAGS}"
    )
  else(cflags)
    set_target_properties( ${TARGET}
	PROPERTIES
	COMPILE_FLAGS "${OpenMP_C_FLAGS}"
    )
  endif(cflags)

  set(ldflags)
  get_target_property(ldflags ${TARGET} LINK_FLAGS)
  if (ldflags)
    set_target_properties( ${TARGET}
	PROPERTIES
	LINK_FLAGS "${ldflags};${OpenMP_C_FLAGS}"
    )
  else(ldflags)
    set_target_properties( ${TARGET}
	PROPERTIES
	LINK_FLAGS "${OpenMP_C_FLAGS}"
    )
  endif(ldflags)
endmacro( target_enable_openmp)

macro( target_add_definitions TARGET)
  set(defs)
  get_target_property(defs ${TARGET} COMPILE_DEFINITIONS)

  if(defs)
    set_target_properties( ${TARGET}
	PROPERTIES
	COMPILE_DEFINITIONS "${defs};${ARGN}"
    )
  else(defs)
    set_target_properties( ${TARGET}
	PROPERTIES
	COMPILE_DEFINITIONS "${ARGN}"
    )
  endif(defs)
endmacro( target_add_definitions)


find_package(CUDA)
find_package(OpenMP)

add_subdirectory(gettime)
add_subdirectory(saxpy)
add_subdirectory(fractal)
