#########################################################################
#                                                                       #
# Copyright (C) 2012-2014 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################


include(ExternalProject)
include(CMakeParseArguments)

# 
# - Add a third party project to this project.
#   It is important to maintain a set of precompiled binaries of each third
#   party component. Each set is not only made up by the combination of CMake
#   configurations (Release vs. Debug vs. ...) and link strategies (Dynamic vs.
#   Static), but also by the Compiler, MPI implementation, and OS. This gives a
#   large number of combinations to worry about. The following functions
#   streamline this. 
# 
# add_external_project_to_repository
#        Maintain a build of third party component with the specified flavours
#
#          add_external_project_to_repository( 
#                NAME                <project name>
#                VERSION             <project version>
#                ARCHIVE             <archive .tar.gz>
#                ARCHIVE_MD5         <md5 checksum of archive>
#                PATCH_COMMAND       <command to patch the source>      
#                CONFIGURE_COMMAND   <command to configure>
#                BUILD_COMMAND       <command to compile the code>
#                INSTALL_COMMAND     <command to install the library>
#                CONFIGURE_OPTIONS   <"COMPILER"|"MPI"|"SPEED"|"OS"|"LINK"> 
#                                      <flavour name> <flag1> [flag2...]
#                                      [<"COMPILER"|...> ... ]
#                YIELD_LIBRARIES     <library1>...
#          )
#
#        Basically, the parameters direct CMake how to build the third party
#        component denoted by NAME and VERSION. The ARCHIVE state frome where
#        the prestine sources can be retrieved and what its MD5 hash should be
#        (the last thing we would want is that an advisary replaces this with
#        his own evil code.) The PATCH_COMMAND, CONFIGURE_COMMAND, BUILD_COMMAND, and
#        INSTALL_COMMAND allows you to specify how to patch, configure, build,
#        and install it, respectively. 
#        
#        The CONFIGURE_COMMAND is ammended by the options given through
#        CONFIGURE_OPTIONS. For each of the flavour types COMPILER, MPI, SPEED,
#        OS, and LINK you have to define the flavours by supplying a name and a
#        set of flags that can be given to the configure command. 
# 
#        All commands and configuration options are sensitive to a number of
#        keywords: 
#          {ROOT}       Is replaced by the full path of the installation
#                       directory.
#          {CC}         Is replaced by the full path to the C compiler that is
#                       used by this project.
#          {CXX}        Is replaced by the full path to the C++ compiler that is
#                       used by this project.
#  
#        As flavour names it also possible to replace them by automatic values:
#          {CurrentPlatform}  Is replaced by the current OS identifier
#          {CurrentCompiler}  Is replaced by the name and version of the
#                             current compiler as is used by this project.
#          {CurrentMPI}       Is replaced by the name and version of the
#                             current MPI implementation as is used by this
#                             project.
#
#        The YIELD_LIBRARIES command can be used import the a library from the
#        third party project into the current project. The library is expected
#        to be in the 'lib' directory of the installation directory.
#        
#        Whether the actual library is rebuild when necessary and where it ends
#        up is determined by the two variables:

#          BM_EXTERNAL_COMPONENTS_REBUILD  Set it ON or OFF. ON means that the
#                             component is rebuild when necessar
#          BM_EXTERNAL_COMPONENTS_DIR  The full path to where the builds of the
#                             third party projects can be found.
#
#        Finally the <NAME>_ROOT variable, where <NAME> is the given name of
#        the external project,  is set to the installation directory.
#
#
# - Configure the settings for all third party projects
# 
# confiure_external_projects
#        Configure the settings for all third party projects
#
#          configure_external_projects()
#
#          Output variables: set the EXTERNAL_PROJECTS_FLAVOUR


# Whether to rebuild external components when their configuration has changed
option( BM_EXTERNAL_COMPONENTS_REBUILD "Whether or not to rebuild external components, like PETSc, HDF5, etc..., if their configuration has changed" ON)

# Location of external components dir
set(BM_EXTERNAL_COMPONENTS_DIR "${PROJECT_BINARY_DIR}/ExternalComponents" CACHE PATH "The path to the directory of prebuilt libraries")

# Determine Compiler identifier
set( BM_COMPILER_ID "${CMAKE_CXX_COMPILER_ID}_${CMAKE_CXX_COMPILER_VERSION}" CACHE STRING "An identifier for the Compiler for this build." )
message(STATUS "Compiler identifier = ${BM_COMPILER_ID}")

# Determine the MPI identifier
set( BM_MPI_ID "${MPI_NAME}_${MPI_VERSION}" CACHE STRING "An identifier for the MPI for this build." )
message(STATUS "MPI identifier = ${BM_MPI_ID}")

# Set Platform identifier
set( BM_PLATFORM_ID "${CMAKE_SYSTEM}" CACHE STRING "An identifier for the platform (OS, etc...) for this is build" )
message(STATUS "Platform identifier = ${BM_PLATFORM_ID}")

macro( configure_external_projects )

   # Autodetect Compiler
   set( flavour_COMPILER "${BM_COMPILER_ID}")

   # Autodetect OS
   set( flavour_OS "${BM_PLATFORM_ID}")
   
   # Autodetect link type
   set( flavour_LINK)
   if (BUILD_SHARED_LIBS)
     set(flavour_LINK "Dynamic")
   else()
     set(flavour_LINK "Static")
   endif()

   # Autodetect SPEED
   set( flavour_SPEED "${CMAKE_BUILD_TYPE}")

   # Autodetect MPI
   set(flavour_MPI "${BM_MPI_ID}")

   # Set cache
   set( BM_EXTERNAL_COMPONENTS_FLAVOUR 
         "${flavour_COMPILER}/${flavour_MPI}/${flavour_SPEED}/${flavour_OS}/${flavour_LINK}"
         CACHE STRING "Flavour mix of third party projects. Format is COMPILER_MPI_SPEED_OS_LINK."
      )
endmacro()



macro( add_external_project_to_repository )

  # Parse parameters
   set(oneValueArgs 
         NAME
         VERSION
         ARCHIVE 
         ARCHIVE_MD5 
   )
   set(multiValueArgs 
         PATCH_COMMAND
         CONFIGURE_COMMAND 
         BUILD_COMMAND 
         INSTALL_COMMAND
         CONFIGURE_OPTIONS
         YIELD_LIBRARIES
   )
   cmake_parse_arguments("extProj" 
                      "" 
                      "${oneValueArgs}"
                      "${multiValueArgs}"
                      ${ARGN}
         )

   # What is the path to the installed library for each configuration?
   set( extProj_ROOT "${BM_EXTERNAL_COMPONENTS_DIR}/${extProj_NAME}/${extProj_VERSION}/${BM_EXTERNAL_COMPONENTS_FLAVOUR}")
   set( ${extProj_NAME}_ROOT "${extProj_ROOT}" )

   # Replaces {XYZ} keywords in the configuration options
   foreach( parameter ${multiValueArgs})
      # Replace the {ROOT} keyword with the installation directory
      string(REPLACE "{ROOT}" "${extProj_ROOT}" extProj_${parameter} "${extProj_${parameter}}")

      # Replace the {CurrentPlatform} keyword with the current platform ID
      string(REPLACE "{CurrentPlatform}" "${BM_PLATFORM_ID}" extProj_${parameter} "${extProj_${parameter}}")

      # Replace the {CurrentCompiler} keyword with the current compiler ID
      string(REPLACE "{CurrentCompiler}" "${BM_COMPILER_ID}" extProj_${parameter} "${extProj_${parameter}}")

      # Replace the {CurrentPlatform} keyword with the path to the C compiler
      string(REPLACE "{CC}" "${CMAKE_C_COMPILER}" extProj_${parameter} "${extProj_${parameter}}")

      # Replace the {CurrentPlatform} keyword with the path to the C++ compiler
      string(REPLACE "{CXX}" "${CMAKE_CXX_COMPILER}" extProj_${parameter} "${extProj_${parameter}}")
 
      # Replace the {CurrentCompiler} keyword with the current compiler ID
      string(REPLACE "{CurrentMPI}" "${BM_MPI_ID}" extProj_${parameter} "${extProj_${parameter}}")
   endforeach()


   # The following while loop parses all CONFIGURE_OPTIONS parameters by extracting the options 
   # assigning them to a CMake variable called 
   #     extProd_CFG_OPT_{flavourType}_{flavourName}
   # where {flavourType} is substituted by one of the flavour types: COMPILER, MPI, SPEED, OS, or LINK
   # and {flavourName} is subtituted by the users name for this particular value.
   # In short, it parses the CONFIGURE_OPTIONS into a dictionary.
   list(LENGTH extProj_CONFIGURE_OPTIONS optionsLength)
   set(cfgOpts "COMPILER" "MPI" "SPEED" "OS" "LINK")
   set(i 0) 
   while( i LESS optionsLength)

      # Type of flavour: COMPILER, MPI, SPEEd, OS, or LINK?
      list(GET extProj_CONFIGURE_OPTIONS "${i}" flavourType )

      # What is the name of the flavour ?
      math(EXPR flavourNameIndex "${i} + 1") 
      list(GET extProj_CONFIGURE_OPTIONS "${flavourNameIndex}" flavourName)

      # Search for the next flavour definition
      math(EXPR nextFlavour "${i}+2")
      while (nextFlavour LESS optionsLength)
        list(GET extProj_CONFIGURE_OPTIONS "${nextFlavour}" item)
        list(FIND cfgOpts "${item}" found)
        if (NOT found EQUAL "-1" )
          break()
        endif()
        math(EXPR nextFlavour "${nextFlavour} + 1")
      endwhile()

      # copy the flags to the variable
      set(extProj_CFG_OPT_${flavourType}_${flavourName} "")
      math(EXPR j "${i} + 2")
      while ( j LESS nextFlavour)
        list(GET extProj_CONFIGURE_OPTIONS "${j}" flag)
        list(APPEND extProj_CFG_OPT_${flavourType}_${flavourName} "${flag}" )
        math(EXPR j "${j} + 1")
      endwhile()

      # Move i to the next flavour 
      set(i ${nextFlavour})
   endwhile()


   # The following foreach loop parses the BM_EXTERNAL_COMPONENTS_FLAVOUR and picks out
   # configuration options that are applicable for the current CMake settings.
   set(configureFlags)
   string(REPLACE "/" ";" flavourMix "${BM_EXTERNAL_COMPONENTS_FLAVOUR}")
   foreach(flavourType ${cfgOpts} )
     
     # Pop first flavour from the current flavour mix
     list(GET flavourMix 0 flavourName)
     list(REMOVE_AT flavourMix 0)

     # Match this with one of the flavours defined in the command
     if (DEFINED extProj_CFG_OPT_${flavourType}_${flavourName})

       # append the flags to the configure flags
       list(APPEND configureFlags "${extProj_CFG_OPT_${flavourType}_${flavourName}}")
     else()
      
       # Send an error to the user/programmer that the it cannot find the configuration flags 
       message(SEND_ERROR "Could not find matching configuration options for flavour ${flavourName} of type ${flavourType} to compile the external project ${extProj_NAME} for the current active flavour mix: ${BM_EXTERNAL_COMPONENTS_FLAVOUR}")
     endif()
     
   endforeach()


   if (BM_EXTERNAL_COMPONENTS_REBUILD)
      set( extProj_srcdir "${PROJECT_BINARY_DIR}/ExternalComponents/build/${extProj_NAME}")

      # Add the extProj as external project for the current extProj configuration
      ExternalProject_Add( ${extProj_NAME}
         PREFIX          "${extProj_ROOT}"
         URL             "${extProj_ARCHIVE}"
         URL_MD5         "${extProj_ARCHIVE_MD5}"
         PATCH_COMMAND   "${extProj_PATCH_COMMAND}"
         SOURCE_DIR      "${extProj_srcdir}"
         CONFIGURE_COMMAND ${extProj_CONFIGURE_COMMAND} ${configureFlags}
         BUILD_COMMAND   ${extProj_BUILD_COMMAND}
         BUILD_IN_SOURCE 1  # Because some libraries may not support out-of-source compilation
         INSTALL_COMMAND ${extProj_INSTALL_COMMAND}

        # Log everything this function does.
         LOG_DOWNLOAD  1
         LOG_UPDATE    1
         LOG_CONFIGURE 1
         LOG_BUILD     1
         LOG_TEST      1
         LOG_INSTALL   1
      ) 

      # The source-build dir are interesting to keep around because it shows
      # explicitly how the library was built. However, the number of files is
      # too large to store 'naked' on an NFS volume. For that reason this
      # directory is converted into a TAR file.
      set( extProj_PostbuildSrc "${extProj_ROOT}/src/${extProj_NAME}-post_build_src.tar.gz")
      add_custom_command( TARGET ${extProj_NAME}
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" 
            ARGS "-E" "tar" "czf" "${extProj_PostbuildSrc}" "."
            WORKING_DIRECTORY "${extProj_srcdir}"
            COMMENT "Packing the files in the build directory of ${extProj_NAME}"
         )
    
   endif()


  # Add the libraries from YIELD_LIBRARIES as targets
   if (BUILD_SHARED_LIBS)
     set(linkType "SHARED")
   else()
     set(linkType "STATIC")
   endif()

   foreach (lib ${extProj_YIELD_LIBRARIES})
      # Add the library as a target
      add_library(${lib} ${linkType} IMPORTED GLOBAL)

      if (BM_EXTERNAL_COMPONENTS_REBUILD)
         add_dependencies(${lib} ${extProj_NAME})
      endif()

      # Set the location of the library file
      set_target_properties( ${lib} PROPERTIES 
         IMPORTED_LOCATION "${extProj_ROOT}/lib/${CMAKE_${linkType}_LIBRARY_PREFIX}${lib}${CMAKE_${linkType}_LIBRARY_SUFFIX}")
   endforeach()

endmacro()
