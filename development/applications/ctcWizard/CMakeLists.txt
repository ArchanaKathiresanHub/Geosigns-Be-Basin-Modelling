cmake_minimum_required( VERSION 3.8 )

if ( Qt5_FOUND )
  set( CMAKE_AUTOMOC ON )
  set( CMAKE_AUTORCC ON )
  include( InstallCTCWizardApplication.cmake )
  add_subdirectory( src )
endif()
