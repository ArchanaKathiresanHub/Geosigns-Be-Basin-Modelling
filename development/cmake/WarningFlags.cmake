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
      add_definitions(-w2 -Wcheck)

      # Disable warning     1: last line of file ends without a newline
      # Disable warning   304: access control not specified ("public" by default)
      # Disable warning   383: value copied to temporary, reference to temporary used
      # Disable warning   424: extra ";" ignored
      # Disable warning   981: operands are evaluated in unspecified order
      # Disable warning  1418: external function definition with no prior declaration
      # Disable warning  2259: non-pointer conversion from "X" to "Y" may lose signficant bitss
      # Disable warning 11074: Inlining inhibited by limit max-size
      # Disable warning 11076: To get full report use -qopt-report=4 -qopt-report-phase ipo
      add_definitions(-diag-disable 1,304,383,424,981,1418,2259,2415,3346,11074,11076)

      # Disable remarks about automatic cpu dispatch
      add_definitions(-diag-disable=cpu-dispatch)

   elseif(CMAKE_COMPILER_IS_GNUCXX)
      # Assume GCC compiler
      add_definitions(-Wall -Wextra)
   else()
      message(WARNING "Don't know the compiler flags to emit all compilation warnings")
   endif()
endif()
