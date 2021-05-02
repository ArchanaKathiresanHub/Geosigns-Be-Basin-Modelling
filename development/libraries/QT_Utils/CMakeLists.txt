#########################################################################
#                                                                       #
# Copyright (C) 2012-2013 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by CGI Information Systems & Management 
# Consultants Pvt Ltd, India.                                           #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

if ( Qt5_FOUND )
   if( BM_USE_INTEL_COMPILER )
      add_definitions( -Wno-invalid-offsetof )
   endif()
   
   set( QT_Utils_LIBNAME QtUtils )
   
   set( CMAKE_AUTOMOC ON )
   
   include_directories( ${QTXLSX_INCLUDE_DIRS} )
   
   create_bm_library( TARGET ${QT_Utils_LIBNAME}
                   LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets
                   INSTALLTARGET )
				   
				   target_include_directories( ${QT_Utils_LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

endif()