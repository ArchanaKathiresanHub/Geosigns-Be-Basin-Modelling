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

option(BM_ENABLE_WARNINGS "Whether to let the compiler emit all warnings" ON)

if (UNIX AND BM_ENABLE_WARNINGS)
   if (BM_USE_INTEL_COMPILER)
      # Enable all warnings and do some static analysis
      add_definitions(-Wall -Wcheck)

      # Disable remarks about automatic cpu dispatch
      add_definitions(-diag-disable=cpu-dispatch)

      # Disable warning 2259: non-pointer conversion from "X" to "Y" may lose signficant bitss
      add_definitions(-diag-disable 2259)

   elseif(CMAKE_COMPILER_IS_GNUCXX)
      # Assume GCC compiler
      add_definitions(-Wall -Wextra)
   else()
      message(WARNING "Don't know the compiler flags to emit all compilation warnings")
   endif()
endif()
