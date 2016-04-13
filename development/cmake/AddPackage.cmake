#########################################################################
#                                                                       #
# Copyright (C) 2012-2016 Shell International Exploration & Production. #
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
#          add_external_package_info( 
#                CAPABILITY          <capability>
#                NAME                <name>
#                VENDOR              <vendor>
#                VERSION             <version>
#                DESCRIPTION         <description>
#                URL                 <url>
#                LICENSE_TYPE        <license-type>
#                LICENSE_FILE        <license-file>
#                REQUIRED_AT         <"Build"|"Test"|"Runtime">
#                [REQUIRED_BY        capability1 [capability2...]]
#                COUNTRY_OF_ORIGIN   <country>
#                SHIPPED             <"Yes"|"No">
#                INCLUSION_TYPE      <"Static Link"|"Dynamic Link"|
#                                     "Separate Executable">
#                USEABLE_STAND_ALONE <"Yes"|"No">
#                CONTAINS_CRYPTO     <"Yes"|"No"|"Unknown">      
#                ECCN                <eccn|"EAR99"|"Unknown">
#                ECCN_EVIDENCE       url1 [url2...]
#        )
#                                    
#        This command adds information about one external package in cmake
#        variables with their name prefixed with <capability>. It is therefore
#        important that <capability> is a valid CMake identifier. The <name>
#        should be the external's product name, <vendor> the company or
#        author(s) that produced it, <version> the version, <description> a
#        short description of the package, and <url> the product's webpage.
#           License information is supplied via the argument <license-type>,
#        that summarizes the kind of license: Commercial, Apache, BSD, etc...,
#        and <license-file> which should point to the actual text file
#        containing the license text.
#           The parameter REQUIRED_AT and the optional parameter REQUIRED_BY
#        store the specific time when the package is needed, and by which
#        other third party package it is required, if applicable.
#           ECCN information is handled through 
#         * COUNTRY_OF_ORIGIN   -- the country  where the package came from
#         * SHIPPED             -- whether it is actually shipped with a
#                                  release 
#         * INCLUSION_TYPE      -- answers the question how the component is
#                                  connected to the application: "Static
#                                  Link", "Dynamic Link", or "Separate
#                                  Executable"
#         * USEABLE_STAND_ALONE -- As it is delivered, can it be used
#                                  separately from the application?
#         * CONTAINS_CRYPTO     -- Whether it contains or performs encryption
#                                  or decryption
#         * ECCN                -- The Export Control Classificaiton Number
#         * ECCN_EVIDENCE       -- One or more URLs pointing to a auditable
#                                  evidence that would be self-explanatory to
#                                  an external auditor
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
<release-info>
<name>Cauldron</name>
<version>${BM_VERSION_STRING}</version>
<third-party>
${EXTERNAL_PACKAGE_XML_LISTING}
</third-party>
</release-info>
"
   )

endmacro( write_external_package_info_file)


# xml_escape Sanitize a string so that it can be used in XML by replacing
# special characters by entities and control characters by just white space
macro( xml_escape outputVariable oldString)
   set(a "${oldString}")
   set(b)
   string(REPLACE "\"" "&quot;" b "${a}")
   string(REPLACE "&" "&amp;" a "${b}")
   string(REPLACE "<" "&lt;" b "${a}")
   string(REPLACE ">" "&gt;" a "${b}")
   
#  Replace all control-characters
   string(REGEX REPLACE "[^ -~\n]" " " b "${a}")  

   set("${outputVariable}" "${b}")
endmacro(xml_escape)
  
# Add an entry to the package listing file
macro( add_to_external_package_listing capability
      name vendor version description url
      licenseType licenseFile
      requiredAt requiredBy
      country shipped inclusionType standAlone crypto eccn eccnEvidence
     )

   xml_escape( nameX "${name}")
   xml_escape( vendorX "${vendor}")
   xml_escape( versionX "${version}")
   xml_escape( urlX "${url}")
   xml_escape( descriptionX "${description}")

   xml_escape( licenseTypeX "${licenseType}")
   
   set(license)
   if (EXISTS "${licenseFile}")
      file(READ "${licenseFile}" license)
      string(REGEX REPLACE "[^ -~\n]" " " license "${license}")
      set(license "<![CDATA[${license}]]>")
   else()
      set(license "License is unavailable")
   endif()

   xml_escape( requiredAtX "${requiredAt}" )

   set(result)
   foreach(r ${requiredBy})
      xml_escape( rX "${r}" )
      set(result "${result}\n      <by>${rX}</by>")
   endforeach()
   set(requiredByX "${result}")

   xml_escape( countryX "${country}" )
   xml_escape( shippedX "${shipped}" )
   xml_escape( inclusionTypeX "${inclusionType}")
   xml_escape( standAloneX "${standAlone}" )
   xml_escape( cryptoX "${crypto}")
   xml_escape( eccnX "${eccn}")

   set(result)
   foreach( evidence ${eccnEvidence})
      xml_escape(evidenceX "${evidence}")
      set(result "${result}\n      <eccn-evidence>${evidenceX}</eccn-evidence>")
   endforeach()
   set(eccnEvidenceX "${result}")

   set(EXTERNAL_PACKAGE_XML_LISTING
"${EXTERNAL_PACKAGE_XML_LISTING}
<component>
   <name>${nameX}</name>
   <vendor>${vendorX}</vendor>
   <version>${versionX}</version>
   <description>${descriptionX}</description>
   <url>${urlX}</url>
   <required>
     <id>${capability}</id>
     <at>${requiredAtX}</at>
     ${requiredByX}
   </required>
   <license>
     <type>${licenseTypeX}</type>
     <contents>${license}</contents>
   </license>
   <export-control>
     <country-of-origin>${countryX}</country-of-origin>
     <shipped>${shippedX}</shipped>
     <inclusion-type>${inclusionTypeX}</inclusion-type>
     <stand-alone>${standAloneX}</stand-alone>
     <crypto>${cryptoX}</crypto>
     <eccn>${eccnX}</eccn>
     ${eccnEvidenceX}
   </export-control>
</component>  
")

endmacro( add_to_external_package_listing )

# Macro that adds external package info
macro( add_external_package_info )
#                CAPABILITY          <capability>
#                NAME                <name>
#                VENDOR              <vendor>
#                VERSION             <version>
#                DESCRIPTION         <description>
#                URL                 <url>
#                LICENSE_TYPE        <license-type>
#                LICENSE_FILE        <license-file>
#                REQUIRED_AT         <"Build"|"Test"|"Runtime">
#                [REQUIRED_BY        capability1 [capability2...]]
#                COUNTRY_OF_ORIGIN   <country>
#                SHIPPED             <"Yes"|"No">
#                INCLUSION_TYPE      <"Static Link"|"Dynamic Link"|
#                                     "Separate Executable">
#                USEABLE_STAND_ALONE <"Yes"|"No">
#                CONTAINS_CRYPTO     <"Yes"|"No"|"Unknown">      
#                ECCN                <eccn|"EAR99"|"Unknown">
#                ECCN_EVIDENCE       url1 [url2...]

# Define parameter names
set(allParameters
      CAPABILITY "Capability that package provides"
      NAME       "Package name"
      VENDOR     "Package vendor"
      VERSION    "Package version"
      DESCRIPTION "Package description"
      URL        "URL of webpage dedicated to the package"
      LICENSE_TYPE "\"Commercial\" for commercial packages. BSD, GPL, Apache, etc... for open source packages"
      LICENSE_FILE "The file path where the license can be found"
      REQUIRED_AT "The moment when package is required: Build, Test, or Runtime"
      REQUIRED_BY "The other third package that requires this package."
      COUNTRY_OF_ORIGIN "The country where the package comes from"
      SHIPPED     "Whether the package is shipped with the release: Yes or No"
      INCLUSION_TYPE "How the package is shipped with the release: Static Link, Dynamic Link, Separate Executable"
      USEABLE_STAND_ALONE "Whether the package can be used separately from the application as it is installed: Yes, No"
      CONTAINS_CRYPTO "Whether the package contains and/or performs any encryption or decryption"
      ECCN        "The Export Control Classification Number"
      ECCN_EVIDENCE "An URL pointing to evidence of the ECCN"
)

# Set 'allParameterNames' to a list of all parameter names.
# Reset the value of 'paramValue_...'
# Set the value of 'paramDescription_...' to the description
set(allParameterNames)
set(nameInsteadOfDescription 1)
foreach (param ${allParameters})
   
   if (nameInsteadOfDescription)
      list(APPEND allParameterNames ${param})
      set( paramValue_${param} )
      set( currentParam ${param} )

      set(nameInsteadOfDescription 0)
   else()
      set( paramDescription_${currentParam} ${param})
      set(nameInsteadOfDescription 1)
   endif()
endforeach()

# Parse all parameters
set(currentParam)
foreach(param ${ARGN})

   # is the parameter one of the parameter names?
   list(FIND allParameterNames ${param} paramIndex)

   if (paramIndex EQUAL "-1")
     # if not: append the value to the current parameter
     list(APPEND "${currentParam}" "${param}" )
   else()
     # if yes: set the current parameter
     set(currentParam paramValue_${param})
   endif()
endforeach()

#Determine capability
if (NOT paramValue_CAPABILITY)
   message(SEND_ERROR "Parameter CAPABILITY is not defined")
endif()
set(capability "${paramValue_CAPABILITY}")


#Store values in Cache
foreach( paramName ${allParameterNames})
   set( ${capability}_${paramName} "${paramValue_${paramName}}" CACHE STRING "${paramDescription_${paramName}}")
   mark_as_advanced( ${capability}_${paramName} )
endforeach()

#Write entry in the external components file
add_to_external_package_listing(
      "${capability}"
      "${${capability}_NAME}" "${${capability}_VENDOR}" "${${capability}_VERSION}"
      "${${capability}_DESCRIPTION}" "${${capability}_URL}" 
      "${${capability}_LICENSE_TYPE}" "${${capability}_LICENSE_FILE}" 
      "${${capability}_REQUIRED_AT}" "${${capability}_REQUIRED_BY}"
      "${${capability}_COUNTRY_OF_ORIGIN}" "${${capability}_SHIPPED}"
      "${${capability}_INCLUSION_TYPE}" "${${capability}_USEABLE_STAND_ALONE}"
      "${${capability}_CONTAINS_CRYPTO}" "${${capability}_ECCN}"
      "${${capability}_ECCN_EVIDENCE}" 
      )      

endmacro( add_external_package_info )


