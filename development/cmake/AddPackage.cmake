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

# 
# - Publish third party component information to an XML file
# From a legal standpoint it is important to track use of third party
# components. For that reason several macros are defined:
# 
# add_external_package_info  
#        Add information for a single external package.
#
#          add_external_package_info( CAPABILITY   <capability>
#                                     NAME         <name>
#                                     VENDOR       <vendor>
#                                     VERSION      <version>
#                                     LICENSE_TYPE <license-type>
#                                     LICENSE_FILE <license-file>
#                                     URL          <url>
#                                     DESCRIPTION  <description>
#                                     DEPENDENCY <"Build"|"Test"|"Runtime">
#                                   )
#                                    
#        This command adds information about one external package in cmake
#        variables with their name prefixed with <capability>. It is therefore
#        important that <capability> is a valid CMake identifier. The <name>
#        should be the external's product name, <vendor> the company or
#        author(s) that produced it, and <version> the version. License
#        information is supplied via the argument <license-type>, that
#        summarizes the kind of license: Commercial, Apache, BSD, etc..., and
#        <license-file> which should point to the actual text file containing
#        the license text.
#   
# write_external_package_info_file
#        Write the external package info to a file.
#
#          write_external_package_info_file( <file> )
#  
#        This command should occur at the end of the primary CMakeLists.txt
#        file. It will write all package information to an XML file and copies
#        the associated XSL stylesheet with it to the build directory.


# Define the EXTERNAL_PACKAGE_XML_LISTING variable in which the package
# information is accumulated
if (NOT DEFINED EXTERNAL_PACKAGE_XML_LISTING)
  set(EXTERNAL_PACKAGE_XML_LISTING )
endif()


# The write_external_package_info writes the resulting XML file with package
# info
macro( write_external_package_info_file fileName)

   file(COPY "ThirdPartyComponentsListStyle.xsl" DESTINATION . )

   file(WRITE "${fileName}"
   "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<?xml-stylesheet type=\"text/xsl\" href=\"ThirdPartyComponentsListStyle.xsl\"?>
<componentlist>
${EXTERNAL_PACKAGE_XML_LISTING}
</componentlist>
"
   )

endmacro( write_external_package_info_file)


# xml_escape Sanitize a string so that it can be used in XML by replacing
# special characters by entities and control characters by just white space
macro( xml_escape outputVariable oldString)
   set(a "${oldString}")
   set(b)
   string(REPLACE "\"" "&quot;" b "${a}")
   string(REPLACE "'" "&apos;" a "${b}")
   string(REPLACE "&" "&amp;" b "${a}")
   string(REPLACE "<" "&lt;" a "${b}")
   string(REPLACE ">" "&gt;" b "${a}")
   
#  Replace all control-characters
   string(REGEX REPLACE "[^ -~\n]" " " a "${b}")  

   set("${outputVariable}" "${a}")
endmacro(xml_escape)
  
# Add an entry to the package listing file
macro( add_to_external_package_listing name vendor version licenseType licenseFile url description)

   xml_escape( name "${name}")
   xml_escape( vendor "${vendor}")
   xml_escape( version "${version}")
   xml_escape( licenseType "${licenseType}")
   
   set(license)
   if (EXISTS "${licenseFile}")
      file(READ "${licenseFile}" license)
      string(REGEX REPLACE "[^ -~\n]" " " license "${license}")
      set(license "<![CDATA[${license}]]>")
   else()
      set(license "License is unavailable")
   endif()

   xml_escape( url "${url}")
   xml_escape( description "${description}")
   xml_escape( dependency "${dependency}" )

   set(EXTERNAL_PACKAGE_XML_LISTING
"${EXTERNAL_PACKAGE_XML_LISTING}
<component>
   <name>${name}</name>
   <vendor>${vendor}</vendor>
   <version>${version}</version>
   <description>${description}</description>
   <dependency>${dependency}</dependency>
   <license>
     <type>${licenseType}</type>
     <contents>${license}</contents>
   </license>
   <url>${url}</url>
</component>  
")

endmacro( add_to_external_package_listing )

# Macro that adds external package info
macro( add_external_package_info )
# CAPABILITY   = Name of capability(e.g. BLAS, GuiToolkit, XMLParser ). Should
#                be CMake identifier
# NAME         = Product Name
# VENDOR       = Company / Organisation / Individual who created it
# VERSION      = Version string
# LICENSE_TYPE = Commerical, BSD, GPL, LGPL, Apache, Creative Commons, etc...
# LICENSE_FILE = File containing the license
# URL          = Optional URL to website of 3rd party component
# DESCRIPTION  = One line summary
# DEPENDENCY   = Runtime / Test / Build

# Parse Parameters
set(capability)
set(name)
set(vendor)
set(version)
set(licenseType)
set(licenseFile)
set(url)
set(description)
set(dependency)  

set(parameterName)
foreach(param ${ARGN})
   if (param STREQUAL CAPABILITY)
      set(parameterName capability)
   elseif (param STREQUAL NAME)
      set(parameterName name)
   elseif(param STREQUAL VENDOR)
      set(parameterName vendor)
   elseif(param STREQUAL VERSION)
      set(parameterName version)
   elseif(param STREQUAL LICENSE_TYPE)
      set(parameterName licenseType)
   elseif(param STREQUAL LICENSE_FILE)
      set(parameterName licenseFile)
   elseif(param STREQUAL URL)
      set(parameterName url)
   elseif(param STREQUAL DESCRIPTION)
      set(parameterName description)
   elseif(param STREQUAL DEPENDENCY)
      set(parameterName dependency)  
   else()
     list(APPEND "${parameterName}" "${param}")
   endif()
endforeach()

#Store values in Cache
set( ${capability}_NAME "${name}" CACHE STRING "Name of ${capability} capability")
set( ${capability}_VENDOR "${vendor}" CACHE STRING "Vendor of ${capability} capability")
set( ${capability}_VERSION "${version}" CACHE STRING "Version of ${capability} capability")
set( ${capability}_LICENSE_TYPE "${licenseType}" CACHE STRING "License type of ${capability} capability")
set( ${capability}_LICENSE_FILE "${licenseFile}" CACHE STRING "License file of ${capability} capability")
set( ${capability}_URL "${url}" CACHE STRING "URL to ${capability} capability")
set( ${capability}_DESCRIPTION "${description}" CACHE STRING "Description of ${capability} capability")
set( ${capability}_DEPENDENCY "${dependency}" CACHE STRING "Dependency type of ${capability} capability")

#Write entry in the external components file
add_to_external_package_listing(
      "${${capability}_NAME}" "${${capability}_VENDOR}" "${${capability}_VERSION}"
      "${${capability}_LICENSE_TYPE}" "${${capability}_LICENSE_FILE}" 
      "${${capability}_URL}" "${${capability}_DESCRIPTION}"
      "${${capability}_DEPENDENCY}"
      )      

endmacro( add_external_package_info )


