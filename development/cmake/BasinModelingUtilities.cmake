#########################################################################
#                                                                       #
# Copyright (C) 2012-2013 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

# Basin Modeling Utilities & Functions

macro(create_bm_library)
  set(prefix BM)
  set(options INSTALLTARGET)
  set(oneValueArgs TARGET)
  set(multiValueArgs LIBRARIES ADD_SOURCES ADD_HEADERS)
  cmake_parse_arguments("${prefix}" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  file(GLOB ALL_HEADERS src/*.h)
  file(GLOB ALL_SOURCES src/*.C src/*.cpp src/*.hpp)

  source_group(include FILES ${ALL_HEADERS} ${BM_ADD_HEADERS})
  source_group(source  FILES ${ALL_SOURCES} ${BM_ADD_SOURCES})

  add_library(${BM_TARGET} ${ALL_SOURCES} ${BM_ADD_SOURCES}
                           ${ALL_HEADERS} ${BM_ADD_HEADERS} )
  target_include_directories(${BM_TARGET} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
  target_link_libraries(${BM_TARGET} ${BM_LIBRARIES})
  set_target_properties( ${BM_TARGET} PROPERTIES FOLDER "${BASE_FOLDER}/${BM_TARGET}" )

  if (BM_INSTALLTARGET)
    install(TARGETS ${BM_TARGET}
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
    )
  endif(BM_INSTALLTARGET)
endmacro(create_bm_library)


# The following function is copied from
# http://stackoverflow.com/questions/10113017/setting-the-msvc-runtime-in-cmake
macro(configure_msvc_runtime FLAVOR)
  if(MSVC)

    # Set compiler options.
    set(variables
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_MINSIZEREL
      CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_MINSIZEREL
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_RELWITHDEBINFO
    )
    if(${FLAVOR} STREQUAL "static")
      message(STATUS
        "MSVC -> forcing use of statically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MD")
          string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
        endif()
      endforeach()
    else()
      message(STATUS
        "MSVC -> forcing use of dynamically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MT")
          string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
        endif()
      endforeach()
    endif()
  endif()
endmacro()

macro( generate_dox DOXYGEN_CONFIG_FILE )
  if (BM_BUILD_DOCS AND DOXYGEN_FOUND)
    message(STATUS "Configure doxygen with ${DOXYGEN_CONFIG_FILE}")
    # generate an identifier according to the corresponding project name
    get_filename_component(id ${CMAKE_CURRENT_BINARY_DIR} NAME)
    string(REPLACE " " "_" id ${id})
    file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc)
    if (UNIX)
      add_custom_target( doc_${id}
        # Override the OUTPUT_DIRECTORY config variable with a trick described in the doxygen FAQ
        COMMAND /bin/bash -c \"( cat ${CMAKE_CURRENT_SOURCE_DIR}/${DOXYGEN_CONFIG_FILE} \; echo 'OUTPUT_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/doc'; ) | ${DOXYGEN_EXECUTABLE} -\"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
        COMMENT "Generating Doxygen documentation with ${DOXYGEN_CONFIG_FILE}"
      )
    endif(UNIX)
    if (WIN32)
      file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${DOXYGEN_CONFIG_FILE} DOXY_CFG_PATH)
      file(TO_NATIVE_PATH ${DOXYGEN_EXECUTABLE} DOXY_EXE_PATH)
      add_custom_target( doc_${id}
        # Override the OUTPUT_DIRECTORY config variable with a trick described in the doxygen FAQ
        COMMAND cmd.exe /c \"( type ${DOXY_CFG_PATH} & echo OUTPUT_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/doc; ) | ${DOXY_EXE_PATH} -\"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
        COMMENT "Generating Doxygen documentation with ${DOXYGEN_CONFIG_FILE}"
      )
    endif(WIN32)
    add_dependencies(doc "doc_${id}")

    set_target_properties( "doc_${id}" PROPERTIES FOLDER "Documentation" )
  endif()
endmacro(generate_dox)

### Generate GUID
macro(new_guid Output)
  # Generate Project GUID
  set( Alphabet "0123456789ABCDEF")
  string( RANDOM LENGTH 8 ALPHABET ${Alphabet} one)
  string( RANDOM LENGTH 4 ALPHABET ${Alphabet} two)
  string( RANDOM LENGTH 4 ALPHABET ${Alphabet} three)
  string( RANDOM LENGTH 4 ALPHABET ${Alphabet} four)
  string( RANDOM LENGTH 12 ALPHABET ${Alphabet} five)

  set(${Output} "${one}-${two}-${three}-${four}-${five}")
endmacro(new_guid)

# Define a macro to get the current date
macro(today RESULT)
   if (UNIX)
      execute_process(COMMAND "date" "+%Y-%m-%d" OUTPUT_VARIABLE ${RESULT})
      string(REGEX REPLACE "(....)-(..)-(..).*" "\\1\\2\\3" ${RESULT} ${${RESULT}})
   elseif(WIN32)
        execute_process(COMMAND "powershell" " get-date -format {yyyy-MMM-dd}" OUTPUT_VARIABLE ${RESULT})
        string(REPLACE "\n" "" ${RESULT} ${${RESULT}})
   else (UNIX)
      message(WARNING "date not implemented.")
      set(${RESULT} "000000")
   endif (UNIX)
endmacro(today)

macro(generate_version_by_git_last_checkin BASE_FILE_FOLDER RESULT)
   if (GIT_FOUND AND EXISTS ${PROJECT_SOURCE_DIR}/../.git )
      # Exctract Spec file version information
      # the commit's SHA1, and whether the building workspace was dirty or not
      # the date of the commit
      execute_process(COMMAND
         "${GIT_EXECUTABLE}" log -1 --format=%ad --date=format:%Y.1%m.1%d ${BASE_FILE_FOLDER}
         WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
         OUTPUT_VARIABLE ${RESULT}
         ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
   else ()
      set(${RESULT} "${BM_VERSION_NUMBER_MAJOR}.0${BM_VERSION_NUMBER_MAJOR}.000")
   endif ()
endmacro(generate_version_by_git_last_checkin)

### Add Nuget package
macro(generate_nuget_pkg)
   set(PKG_NAME)
   set(PKG_VERSION)
   set(PKG_OWNER)
   set(PKG_DESCRIPTION)
   set(PKG_RELEASE_NOTES)
   set(PKG_TAGS)
   set(PKG_COPYRIGHT)
   set(AdditionalHeaders)
   set(AdditionalLibs)
   set(PKG_TEMPLATE_NUSPEC "NugetPkgTemplate.nuspec.cmake")

   # parse parameters
  set(parameterName)
  set(parameterType)
  foreach(param ${ARGN})
   #      MESSAGE(STATUS "C# API parameter: ${param}")
      if (param STREQUAL PKG_NAME)
         set(parameterName PKG_NAME)
       set(parameterType Atom)
      elseif(param STREQUAL PKG_NAMESPACE)
         set(parameterName PKG_NAMESPACE)
         set(parameterType Atom)
      elseif(param STREQUAL PKG_VERSION)
        set(parameterName  PKG_VERSION)
       set(parameterType Atom)
      elseif(param STREQUAL PKG_OWNER)
        set(parameterName  PKG_OWNER)
       set(parameterType Atom)
      elseif(param STREQUAL PKG_DESCRIPTION)
        set(parameterName  PKG_DESCRIPTION)
       set(parameterType Atom)
      elseif(param STREQUAL PKG_RELEASE_NOTES)
         set(parameterName PKG_RELEASE_NOTES)
       set(parameterType Atom)

        set(parameterName  PKG_TAGS)
       set(parameterType Atom)
      elseif(param STREQUAL PKG_COPYRIGHT)
         set(parameterName  PLG_COPYRIGHT)
       set(parameterType Atom)
      elseif(param STREQUAL PKG_LIBRARIES) # native libraries which must be bundled with nuget package
        set(parameterName  AdditionalLibs)
       set(parameterType Sequence)
      elseif(param STREQUAL PKG_HEADERS)  # native library includes which must be bundled with nuget package
        set(parameterName  AdditionalHeaders)
       set(parameterType Sequence)
      elseif(param STREQUAL PKG_TEMPLATE_NUSPEC)
        set(parameterName  PKG_TEMPLATE_NUSPEC)
         set(parameterType Atom)
    elseif(parameterType STREQUAL Atom)
       set(${parameterName} ${param})
    elseif(parameterType STREQUAL Sequence)
       list(APPEND ${parameterName} ${param})
      endif()
  endforeach(param)

   # Extract project name to use it in nuget package creation
   get_filename_component(PROJ_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

   if (BUILD_SHARED_LIBS)
      set(LIB_SFX so)
   else (BUILD_SHARED_LIBS)
      set(LIB_SFX a)
   endif (BUILD_SHARED_LIBS)

   if (UNIX)
      execute_process(COMMAND "/bin/sh" "${CMAKE_SOURCE_DIR}/cmake/getos2" "--arch" "--ver" OUTPUT_VARIABLE BLD_ARCH)
   else (UNIX) # Windows
      if (MSVC14)
         set(BLD_ARCH x64_vc14)
      else ()
         message(FATAL_ERROR "Generate nuget package: Unsupported version of VisualStudio")
      endif ()
   endif (UNIX)

   # Create list of additional libraries for nuget package
   if (AdditionalLibs)
      foreach(lib ${AdditionalLibs})
         if (NOT TARGET_NAME ) # assume that first lib in the list is target for this project
            set(TARGET_NAME ${lib})
         endif ()
         get_filename_component(libPath ${lib} DIRECTORY)
         get_filename_component(libFile ${lib} NAME)
         get_filename_component(fullLibPath "${CMAKE_CURRENT_BINARY_DIR}/${libPath}" ABSOLUTE)

         file(TO_NATIVE_PATH ${fullLibPath}/lib${libFile}.${LIB_SFX} fullFilePathNative)
         set(PKG_LIBS "${PKG_LIBS}    <file src=\"${fullFilePathNative}\" target=\"lib/\"/>\n")
      endforeach()
   else ()
      set(PKG_LIBS)
   endif ()

   # Create list of additional headers for nuget package
   if (AdditionalHeaders)
      foreach(doth ${AdditionalHeaders})
         get_filename_component(fullFilePath "${CMAKE_CURRENT_SOURCE_DIR}/src/${doth}" ABSOLUTE)
         file(TO_NATIVE_PATH ${fullFilePath} fullFilePathNative)

         set(PKG_HEADERS "${PKG_HEADERS}    <file src=\"${fullFilePathNative}\" target=\"include/\"/>\n")
      endforeach()
   else ()
      set(PKG_HEADERS)
   endif ()

   file(TO_NATIVE_PATH ${CMAKE_BINARY_DIR} TARGET_FILE_PATH)
   set(NUGET_SPEC_FILE_NAME  ${PKG_NAMESPACE}.${BLD_ARCH}.${PKG_NAME}.nuspec)

   configure_file(${PROJECT_SOURCE_DIR}/cmake/${PKG_TEMPLATE_NUSPEC} ${CMAKE_BINARY_DIR}/${NUGET_SPEC_FILE_NAME})

   # Copy package icon
   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/${PKG_NAME}.png ${CMAKE_BINARY_DIR} COPYONLY)

   # Call nuget packaging at the end of this project build
   add_custom_command(TARGET ${TARGET_NAME}
                      POST_BUILD
                      COMMAND ${NUGET_EXECUTABLE} "pack" "${NUGET_SPEC_FILE_NAME}"
                      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
                     )
endmacro(generate_nuget_pkg)

### Add C# API
# Each C++ library for which C# API will be generated
# must has ${CSPROJ_NAME}.i file in src folder, ${CSPROJ_NAME}.png file with icon in doc folder
macro(generate_csharp_api)
   set(CSPROJ_NAME)
   set(CSPROJ_NAMESPACE)
   set(CSPROJ_ASSEMBLY_VERSION)
   set(CSPROJ_ASSEMBLY_OWNER)
   set(CSPROJ_ASSEMBLY_DESCRIPTION)
   set(CSPROJ_ASSEMBLY_RELEASE_NOTES)
   set(CSPROJ_ASSEMBLY_TAGS)
   set(CSPROJ_ASSEMBLY_COPYRIGHT)
   set(ProjectLinkLibraries)
   set(UnitTestsFileList)
   set(ExtraDependencies)
   set(AdditionalHeaders)
   set(AdditionalLibs)

   # parse parameters
  set(parameterName)
  set(parameterType)
  foreach(param ${ARGN})
#      MESSAGE(STATUS "C# API parameter: ${param}")
      if (param STREQUAL CSPROJ_NAME)
        set(parameterName CSPROJ_NAME)
       set(parameterType Atom)
      elseif(param STREQUAL CSPROJ_NAMESPACE)
        set(parameterName CSPROJ_NAMESPACE)
       set(parameterType Atom)
      elseif(param STREQUAL CSPROJ_ASSEMBLY_VERSION)
        set(parameterName CSPROJ_ASSEMBLY_VERSION)
       set(parameterType Atom)
      elseif(param STREQUAL CSPROJ_ASSEMBLY_OWNER)
        set(parameterName CSPROJ_ASSEMBLY_OWNER)
       set(parameterType Atom)
      elseif(param STREQUAL CSPROJ_ASSEMBLY_DESCRIPTION)
        set(parameterName CSPROJ_ASSEMBLY_DESCRIPTION)
       set(parameterType Atom)
      elseif(param STREQUAL CSPROJ_ASSEMBLY_RELEASE_NOTES)
        set(parameterName CSPROJ_ASSEMBLY_RELEASE_NOTES)
       set(parameterType Atom)
      elseif(param STREQUAL CSPROJ_ASSEMBLY_TAGS)
        set(parameterName CSPROJ_ASSEMBLY_TAGS)
       set(parameterType Atom)
      elseif(param STREQUAL CSPROJ_ASSEMBLY_COPYRIGHT)
        set(parameterName CSPROJ_ASSEMBLY_COPYRIGHT)
       set(parameterType Atom)
      elseif(param STREQUAL CSPROJ_LIBRARIES)  # list of libraries with which swig generated C++ dll must be linked
        set(parameterName ProjectLinkLibraries)
       set(parameterType Sequence)
      elseif(param STREQUAL CSPROJ_ADDITIONAL_LIBRARIES) # native libraries which must be bundled with nuget package
        set(parameterName AdditionalLibs)
       set(parameterType Sequence)
      elseif(param STREQUAL CSPROJ_ADDITIONAL_HEADERS)  # native library includes which must be bundled with nuget package
        set(parameterName AdditionalHeaders)
       set(parameterType Sequence)
      elseif(param STREQUAL CSHARP_UNIT_TESTS_SRC) # list of files with C# unit tests
         set(parameterName UnitTestsFileList)
       set(parameterType Sequence)
      elseif(param STREQUAL CSPROJ_EXTRA_DEPS) #
         set(parameterName ExtraDependencies)
       set(parameterType Sequence)
    elseif(parameterType STREQUAL Atom)
       set(${parameterName} ${param})
    elseif(parameterType STREQUAL Sequence)
       list(APPEND ${parameterName} ${param})
      endif()
  endforeach(param)

  if (MSVC)

      configure_file(${PROJECT_SOURCE_DIR}/cmake/version.rc.cmake version.rc)
      configure_file(${PROJECT_SOURCE_DIR}/cmake/AssemblyInfo.cs.cmake csharp/Properties/AssemblyInfo.cs)

      # Extract project name to use it in nuget package creation
      get_filename_component(PROJ_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

      # Create list of additional libraries for nuget package
      if (AdditionalLibs)
         foreach(lib ${AdditionalLibs})
            get_filename_component(libPath ${lib} DIRECTORY)
            get_filename_component(libFile ${lib} NAME)
            get_filename_component(fullLibPath "${CMAKE_CURRENT_BINARY_DIR}/${libPath}" ABSOLUTE)

            file(TO_NATIVE_PATH ${fullLibPath}/Release/${libFile} fullFilePathNative)
            set(ADDITIONAL_LIBS "${ADDITIONAL_LIBS}    <file src=\"${fullFilePathNative}\" target=\"lib\\native\\x64\\Release\"/>\n")

            file(TO_NATIVE_PATH ${fullLibPath}/Debug/${libFile} fullFilePathNative)
            set(ADDITIONAL_LIBS "${ADDITIONAL_LIBS}    <file src=\"${fullFilePathNative}\" target=\"lib\\native\\x64\\Debug\"/>\n")

            string(REPLACE ".lib" ".pdb" libFilePDB ${libFile})
            file(TO_NATIVE_PATH ${fullLibPath}/Debug/${libFilePDB} fullFilePathNative)
            set(ADDITIONAL_LIBS "${ADDITIONAL_LIBS}    <file src=\"${fullFilePathNative}\" target=\"lib\\native\\x64\\Debug\"/>\n")
         endforeach()
      else ()
         set(ADDITIONAL_LIBS)
      endif ()

      # Create list of additional headers for nuget package
      if (AdditionalHeaders)
         foreach(doth ${AdditionalHeaders})
            get_filename_component(fullFilePath "${CMAKE_CURRENT_SOURCE_DIR}/src/${doth}" ABSOLUTE)
            file(TO_NATIVE_PATH ${fullFilePath} fullFilePathNative)

            set(ADDITIONAL_HEADERS "${ADDITIONAL_HEADERS}    <file src=\"${fullFilePathNative}\" target=\"lib\\native\\include\"/>\n")
         endforeach()
      else ()
         set(ADDITIONAL_HEADERS)
      endif ()
      file(TO_NATIVE_PATH ${CMAKE_BINARY_DIR} TARGET_FILE_PATH)
      configure_file(${PROJECT_SOURCE_DIR}/cmake/CSharpSwigAPITemplate.nuspec.cmake  ${CMAKE_BINARY_DIR}/${CSPROJ_NAME}.nuspec)

      # Copy assembly icon
      configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/${CSPROJ_NAME}.png ${CMAKE_BINARY_DIR} COPYONLY)
      configure_file(${PROJECT_SOURCE_DIR}/cmake/CSharpSwigAPITemplate.targets.cmake ${CMAKE_BINARY_DIR}/${CSPROJ_NAMESPACE}.${CSPROJ_NAME}.targets)

      set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/csharp)

      set(SWIG_INP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/${CSPROJ_NAME}.i)

      if (NOT EXISTS "${SWIG_INP_FILE}")
         MESSAGE(STATUS "File ${CMAKE_CURRENT_SOURCE_DIR}/src/${CSPROJ_NAME}.i does not exist. Configure swig to use generated .i file")
         set(SWIG_INP_FILE ${CMAKE_CURRENT_BINARY_DIR}/${CSPROJ_NAME}.i)
         set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${CSPROJ_NAME}.i PROPERTIES GENERATED TRUE)
      endif ()

      set_source_files_properties( ${SWIG_INP_FILE}
            PROPERTIES SWIG_FLAGS "-namespace;${CSPROJ_NAMESPACE};-I${CMAKE_CURRENT_BINARY_DIR}"
            CPLUSPLUS ON
      )
#            PROPERTIES SWIG_FLAGS "-namespace;${CSPROJ_NAMESPACE}.${CSPROJ_NAME};-I${CMAKE_CURRENT_BINARY_DIR}"

      if( ExtraDependencies )
         MESSAGE(STATUS "Set extra dependencies ${ExtraDependencies} for ${CSPROJ_NAME}")
         set(SWIG_MODULE_${CSPROJ_NAME}_EXTRA_DEPS ${ExtraDependencies})
      endif ()

      swig_add_library(${CSPROJ_NAME} TYPE MODULE LANGUAGE csharp SOURCES ${SWIG_INP_FILE} ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
      swig_link_libraries(${CSPROJ_NAME} ${ProjectLinkLibraries})

      # Before C# generation, remove all existing files. The directory should be empty before generation
      add_custom_command( TARGET ${CSPROJ_NAME}
         PRE_BUILD
         COMMAND ${CMAKE_COMMAND} ARGS "-E" "remove" "*.cs"
         WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/csharp"
      )

      # Visual studio should reload the project after C# generation
      add_custom_command(TARGET ${CSPROJ_NAME}
         PRE_LINK
         COMMAND ${CMAKE_COMMAND} ARGS "-E" "touch_nocreate" "${CSPROJ_NAMESPACE}.${CSPROJ_NAME}.csproj"
         WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/csharp"
      )

      # Generate C# project to compile generated C# files.
      new_guid(Guid)
      configure_file(${PROJECT_SOURCE_DIR}/cmake/CSharpSwigAPITemplate.csproj.cmake ${CMAKE_CURRENT_BINARY_DIR}/csharp/${CSPROJ_NAMESPACE}.${CSPROJ_NAME}.csproj)

      include_external_msproject(${CSPROJ_NAMESPACE}.${CSPROJ_NAME} ${CMAKE_CURRENT_BINARY_DIR}/csharp/${CSPROJ_NAMESPACE}.${CSPROJ_NAME}.csproj
         TYPE "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}" # This GUID is a Windows C# project (see also http://msdn.microsoft.com/en-us/library/hb23x61k(v=vs.80).aspx )
         PLATFORM "${BM_WINDOWS_PLATFORM}"
         GUID "${Guid}"
         ${CSPROJ_NAME}
      )

      ###### Installation
      install(TARGETS ${CSPROJ_NAME}
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION bin
      )

      install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/csharp/Debug/${CSPROJ_NAMESPACE}.${CSPROJ_NAME}.dll
         DESTINATION  bin
         CONFIGURATIONS Debug
      )

      install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/csharp/Release/${CSPROJ_NAMESPACE}.${CSPROJ_NAME}.dll
         DESTINATION  bin
         CONFIGURATIONS Release
      )

      configure_file(${PROJECT_SOURCE_DIR}/cmake/AssemblyInfo.cs.test.cmake csharp-test/Properties/AssemblyInfo.cs)

      add_csharp_unittest(NAME ${CSPROJ_NAMESPACE}.${CSPROJ_NAME}.Test
         PLATFORM ${BM_WINDOWS_PLATFORM}
         DIRECTORY csharp-test
         TEST_SOURCES ${UnitTestsFileList}
         DEPENDS ${CSPROJ_NAMESPACE}.${CSPROJ_NAME}
         TESTLIST "Test"
         TFS_SERVER_URL ${BM_TFS_SERVER_URL}
         TFS_BUILD_NUMBER ${BM_TFS_BUILD_NUMBER}
         TFS_PROJECT_NAME ${BM_TFS_PROJECT_NAME}
         DEPLOYMENT_ITEMS "../\$(Configuration)/${CSPROJ_NAME}.dll"
         PROJECT_REFERENCE ${CSPROJ_NAMESPACE}.${CSPROJ_NAME}
         "${Guid}"
         "${CMAKE_CURRENT_BINARY_DIR}/csharp/${CSPROJ_NAMESPACE}.${CSPROJ_NAME}.csproj"
      )

  endif(MSVC)

endmacro( generate_csharp_api )

### Add C# Unit tests
macro(add_csharp_unittest )

  set(TestProjectName)
  set(Platform)
  set(Directory)
  set(Dependencies)
  set(TestList)
  set(TfsServerUrl)
  set(TfsBuildNumber)
  set(TfsProjectName)
  set(DeploymentItems)
  set(ProjectReferenceName)
  set(ProjectReferenceGuid)
  set(ProjectReferencePath)
  set(TestSourceFilesList)

  # parse parameters
  set(parameterName)
  set(parameterType)
  foreach(param ${ARGN})
      if (param STREQUAL NAME)
        set(parameterName TestProjectName)
       set(parameterType Atom)
      elseif(param STREQUAL PLATFORM)
       set(parameterName Platform)
       set(parameterType Atom)
    elseif(param STREQUAL DIRECTORY)
       set(parameterName Directory)
       set(parameterType Atom)
    elseif(param STREQUAL DEPENDS)
       set(parameterName Dependencies)
       set(parameterType Sequence)
    elseif(param STREQUAL TESTLIST)
       set(parameterName TestList)
       set(parameterType Sequence)
    elseif(param STREQUAL TFS_SERVER_URL)
       set(parameterName TfsServerUrl)
       set(parameterType Atom)
    elseif(param STREQUAL TFS_BUILD_NUMBER)
       set(parameterName TfsBuildNumber)
       set(parameterType Atom)
    elseif(param STREQUAL TFS_PROJECT_NAME)
       set(parameterName TfsProjectName)
       set(parameterType Atom)
    elseif(param STREQUAL DEPLOYMENT_ITEMS)
       set(parameterName DeploymentItems)
       set(parameterType Sequence)
    elseif(param STREQUAL PROJECT_REFERENCE)
       set(parameterType ProjectReference)
       set(parameterName ProjectReferenceName)
    elseif(param STREQUAL TEST_SOURCES)
       set(parameterName TestSourceFilesList)
       set(parameterType Sequence)
    elseif(parameterType STREQUAL Atom)
       set(${parameterName} ${param})
    elseif(parameterType STREQUAL Sequence)
       list(APPEND ${parameterName} ${param})
    elseif(parameterType STREQUAL ProjectReference)
       set(${parameterName} ${param})
       set(prn "ProjectReferenceName") # This and the two lines below are a workaround for CMake's silly if-statemetn
       set(prg "ProjectReferenceGuid")
       set(prp "ProjectReferencePath")
       if (parameterName STREQUAL prn)
         set(parameterName ProjectReferenceGuid)
       elseif (parameterName STREQUAL prg)
         set(parameterName ProjectReferencePath)
       elseif (parameterName STREQUAL prp)
         set(parameterName)
       endif()
    endif()
  endforeach(param)

  if (MSVC)
      new_guid(Guid)
      set(ProjectReferences)
      if (ProjectReferenceName AND ProjectReferencePath AND ProjectReferenceGuid)
         file(TO_NATIVE_PATH "${ProjectReferencePath}" ProjectReferencePath)
       set(ProjectReferences "<ProjectReference Include='${ProjectReferencePath}'><Project>{${ProjectReferenceGuid}}</Project><Name>${ProjectReferenceName}</Name></ProjectReference>")
    endif()

    set (DeploymentItemsList)
      foreach (item ${DeploymentItems})
    set( NativePathToFile)
      file( TO_NATIVE_PATH "${item}" NativePathToFile)
      set (DeploymentItemsList "${DeploymentItemsList}<None Include=\"${NativePathToFile}\">\n      <CopyToOutputDirectory>Always</CopyToOutputDirectory>\n    </None>\n" )
     endforeach(item)
    if (TestSourceFilesList)
       set(PrefixedTestSourceFilesList)
       foreach(fitem ${TestSourceFilesList} )
         set(NativeFullPathToFile)
        file( TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${fitem}" NativeFullPathToFile)
        list(APPEND PrefixedTestSourceFilesList "${NativeFullPathToFile}")
      endforeach(fitem)
      set(FileListForCompilation "${PrefixedTestSourceFilesList}")
    else(TestSourceFilesList)
        set(FileListForCompilation "*.cs")
    endif(TestSourceFilesList)

      configure_file("${PROJECT_SOURCE_DIR}/cmake/CSharpTestProjectTemplate.csproj.cmake" "${Directory}/${TestProjectName}.csproj" @ONLY)

    ## Write the local.testsettings file
    set(TestSettings "${CMAKE_CURRENT_BINARY_DIR}/${Directory}/local.testsettings")
    file(WRITE "${TestSettings}" "<?xml version='1.0' encoding='UTF-8'?><TestSettings name='Local' id='4fd90fa6-1a9f-49c6-a25d-79db40300acb' xmlns='http://microsoft.com/schemas/VisualStudio/TeamTest/2010'> <Description>These are default test settings for a local test run.</Description><Deployment>")

      foreach (item ${DeploymentItems})
         file(APPEND "${TestSettings}" "<DeploymentItem filename='${item}' />")
      endforeach(item)

      if ( Platform STREQUAL x64 )
       set( TestSettingsExecutionParams hostProcessPlatform="MSIL" )
         # Adding this attribute let's MSTest test 64-bit code
         # Note, see http://www.cmake.org/Wiki/CMake/Language_Syntax for the weird rules on quoting in CMake
      elseif ( Platform STREQUAL Win32 )
         set( TestSettingsExecutionParams )
         # Clearing that attribute, forces MSTest to run in 32-bit mode.
      else()
         message("Error: Cannot run MSTest on other platforms than x64 and Win32")
         set( TestSettingsExecutionParams )
      endif()
      file(APPEND "${TestSettings}" "</Deployment><Execution ${TestSettingsExecutionParams} ><TestTypeSpecific /><AgentRule name='LocalMachineDefaultRole'/></Execution></TestSettings>")

     ## Include the generated C# project
    include_external_msproject("${TestProjectName}" "${CMAKE_CURRENT_BINARY_DIR}/${Directory}/${TestProjectName}.csproj"
      TYPE "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}" # This GUID is a Windows C# project (see also http://msdn.microsoft.com/en-us/library/hb23x61k(v=vs.80).aspx )
      PLATFORM "${Platform}"
      GUID "${Guid}"
      ${Dependencies}
    )

      # Add the test, and publish results if the necessary information is available

    if (TfsServerUrl AND TfsBuildNumber AND TfsProjectName)
        add_test(NAME "${TestProjectName}"
         COMMAND "${MSTEST}"
          "/testcontainer:${Directory}/bin/$<CONFIGURATION>/${TestProjectName}.dll"
          "/testsettings:${TestSettings}"
          "/test:${TestList}"
          "/publish:${TfsServerUrl}"
          "/publishbuild:${TfsBuildNumber}"
          "/flavor:$<CONFIGURATION>"
          "/platform:${Platform}"
          "/teamproject:${TfsProjectName}"
      )
    else()
        add_test(NAME "${TestProjectName}"
         COMMAND "${MSTEST}"
          "/testcontainer:${Directory}/bin/$<CONFIGURATION>/${TestProjectName}.dll"
          "/testsettings:${TestSettings}"
          "/test:${TestList}"
      )
    endif()
  endif(MSVC)
endmacro(add_csharp_unittest)

###


# Local Variables:
# mode: cmake
# cmake-tab-width: 4
# tab-width: 4
# End:
