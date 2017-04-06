//
// version.rc - automaticaly generated. 
// Please edit ${CMAKE_CURRENT_SOURCE_DIR}/src/version.rc.cmake
//

#include <winver.h>

VS_VERSION_INFO VERSIONINFO
  FILEVERSION    ${BM_VERSION_NUMBER_MAJOR},${BM_VERSION_NUMBER_MINOR},${BM_BUILD_NUMBER},${BM_REVISION}
  PRODUCTVERSION ${BM_VERSION_NUMBER_MAJOR},${BM_VERSION_NUMBER_MINOR},${BM_BUILD_NUMBER},${BM_REVISION}
  FILEFLAGSMASK  VS_FFI_FILEFLAGSMASK
  FILEFLAGS      0
  FILEOS         VOS__WINDOWS32
  FILETYPE       VFT_DLL
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904b0"
        BEGIN
            VALUE "FileDescription", "Project3dAPI library\0"
            VALUE "ProductVersion", "${BM_VERSION_STRING} Build ${BM_BUILD_NUMBER}\0"
            VALUE "FileVersion", "${BM_VERSION_NUMBER} Build ${BM_BUILD_NUMBER}\0"
            VALUE "InternalName", "Project3dAPI\0"
            VALUE "OriginalFilename", "Project3dAPI.dll\0"
            VALUE "CompanyName", "Shell International Exploration & Production\0"
            VALUE "LegalCopyright", "Copyright (C) Shell International Exploration & Production 2012-2017\0"
            VALUE "Licence", "Shell\0"
            VALUE "ProductName", "Basin Modeling\0"
	    VALUE "Comment", "\0"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1200
    END
END

/*
VERSIONINFO Resource

The VERSIONINFO resource-definition statement creates a version-information
resource. The resource contains such information about the file as its
version number, its intended operating system, and its original filename.
The resource is intended to be used with the Version Information functions.

versionID VERSIONINFO fixed-info  { block-statement...}

versionID
    Version-information resource identifier. This value must be 1.

fixed-info
    Version information, such as the file version and the intended operating
    system. This parameter consists of the following statements.


    Statement 		Description
    --------------------------------------------------------------------------
    FILEVERSION
    version 		Binary version number for the file. The version
			consists of two 32-bit integers, defined by four
			16-bit integers. For example, "FILEVERSION 3,10,0,61"
			is translated into two doublewords: 0x0003000a and
			0x0000003d, in that order. Therefore, if version is
			defined by the DWORD values dw1 and dw2, they need
			to appear in the FILEVERSION statement as follows:
			HIWORD(dw1), LOWORD(dw1), HIWORD(dw2), LOWORD(dw2).
    PRODUCTVERSION
    version 		Binary version number for the product with which the
			file is distributed. The version parameter is two
			32-bit integers, defined by four 16-bit integers.
			For more information about version, see the
			FILEVERSION description.
    FILEFLAGSMASK
    fileflagsmask 	Bits in the FILEFLAGS statement are valid. If a bit
			is set, the corresponding bit in FILEFLAGS is valid.
    FILEFLAGSfileflags 	Attributes of the file. The fileflags parameter must
			be the combination of all the file flags that are
			valid at compile time. For 16-bit Windows, this
			value is 0x3f.
    FILEOSfileos 	Operating system for which this file was designed.
			The fileos parameter can be one of the operating
			system values given in the Remarks section.
    FILETYPEfiletype 	General type of file. The filetype parameter can be
			one of the file type values listed in the Remarks
			section.
    FILESUBTYPE
    subtype 		Function of the file. The subtype parameter is zero
			unless the type parameter in the FILETYPE statement
			is VFT_DRV, VFT_FONT, or VFT_VXD. For a list of file
			subtype values, see the Remarks section.

block-statement
    Specifies one or more version-information blocks. A block can contain
    string information or variable information. For more information, see
    StringFileInfo Block or VarFileInfo Block.

Remarks

To use the constants specified with the VERSIONINFO statement, you must
include the Winver.h or Windows.h header file in the resource-definition file.

The following list describes the parameters used in the VERSIONINFO statement:

fileflags
    A combination of the following values.

    Value 		Description

    VS_FF_DEBUG 	File contains debugging information or is compiled
			with debugging features enabled.
    VS_FF_PATCHED 	File has been modified and is not identical to the
			original shipping file of the same version number.
    VS_FF_PRERELEASE 	File is a development version, not a commercially
			released product.
    VS_FF_PRIVATEBUILD 	File was not built using standard release procedures.
			If this value is given, the StringFileInfo block must
			contain a PrivateBuild string.
    VS_FF_SPECIALBUILD 	File was built by the original company using standard
			release procedures but is a variation of the standard
			file of the same version number. If this value is
			given, the StringFileInfo block must contain a
			SpecialBuild string.

fileos
    One of the following values.

    Value 		Description

    VOS_UNKNOWN 	The operating system for which the file was designed
			is unknown.
    VOS_DOS 		File was designed for MS-DOS.
    VOS_NT 		File was designed for Windows Server 2003 family,
			Windows XP, Windows 2000, or Windows NT.
    VOS__WINDOWS16 	File was designed for 16-bit Windows.
    VOS__WINDOWS32 	File was designed for 32-bit Windows.
    VOS_DOS_WINDOWS16 	File was designed for 16-bit Windows running with
			MS-DOS.
    VOS_DOS_WINDOWS32 	File was designed for 32-bit Windows running with
			MS-DOS.
    VOS_NT_WINDOWS32 	File was designed for Windows Server 2003 family,
			Windows XP, Windows 2000, or Windows NT.

    The values 0x00002L, 0x00003L, 0x20000L and 0x30000L are reserved.

filetype
    One of the following values.

    Value 		Description

    VFT_UNKNOWN 	File type is unknown.
    VFT_APP 		File contains an application.
    VFT_DLL 		File contains a dynamic-link library (DLL).
    VFT_DRV 		File contains a device driver. If filetype is
			VFT_DRV, subtype contains a more specific
			description of the driver.
    VFT_FONT 		File contains a font. If filetype is VFT_FONT,
			subtype contains a more specific description of the
			font.
    VFT_VXD 		File contains a virtual device.
    VFT_STATIC_LIB 	File contains a static-link library.

    All other values are reserved for use by Microsoft.

subtype
    Additional information about the file type.

    If filetype specifies VFT_DRV, this parameter can be one of the
    following values.

    Value 			Description

    VFT2_UNKNOWN 		Driver type is unknown.
    VFT2_DRV_COMM 		File contains a communications driver.
    VFT2_DRV_PRINTER 		File contains a printer driver.
    VFT2_DRV_KEYBOARD 		File contains a keyboard driver.
    VFT2_DRV_LANGUAGE 		File contains a language driver.
    VFT2_DRV_DISPLAY 		File contains a display driver.
    VFT2_DRV_MOUSE 		File contains a mouse driver.
    VFT2_DRV_NETWORK 		File contains a network driver.
    VFT2_DRV_SYSTEM 		File contains a system driver.
    VFT2_DRV_INSTALLABLE 	File contains an installable driver.
    VFT2_DRV_SOUND 		File contains a sound driver.
    VFT2_DRV_VERSIONED_PRINTER 	File contains a versioned printer driver.

    If filetype specifies VFT_FONT, this parameter can be one of the
    following values.

    Value 		Description

    VFT2_UNKNOWN 	Font type is unknown.
    VFT2_FONT_RASTER 	File contains a raster font.
    VFT2_FONT_VECTOR 	File contains a vector font.
    VFT2_FONT_TRUETYPE 	File contains a TrueType font.

    If filetype specifies VFT_VXD, this parameter must be the virtual-device
    identifier included in the virtual-device control block.

    All subtype values not listed here are reserved for use by Microsoft.

langID
    One of the following language codes.

    Code 	Language 		Code 	Language

    0x0401 	Arabic 			0x0415 	Polish
    0x0402 	Bulgarian 		0x0416 	Portuguese (Brazil)
    0x0403 	Catalan 		0x0417 	Rhaeto-Romanic
    0x0404 	Traditional Chinese 	0x0418 	Romanian
    0x0405 	Czech 			0x0419 	Russian
    0x0406 	Danish 			0x041A 	Croato-Serbian (Latin)
    0x0407 	German 			0x041B 	Slovak
    0x0408 	Greek 			0x041C 	Albanian
    0x0409 	U.S. English 		0x041D 	Swedish
    0x040A 	Castilian Spanish 	0x041E 	Thai
    0x040B 	Finnish 		0x041F 	Turkish
    0x040C 	French 			0x0420 	Urdu
    0x040D 	Hebrew 			0x0421 	Bahasa
    0x040E 	Hungarian 		0x0804 	Simplified Chinese
    0x040F 	Icelandic 		0x0807 	Swiss German
    0x0410 	Italian 		0x0809 	U.K. English
    0x0411 	Japanese 		0x080A 	Mexican Spanish
    0x0412 	Korean 			0x080C 	Belgian French
    0x0413 	Dutch 			0x0C0C 	Canadian French
    0x0414 	Norwegian � Bokmal 	0x100C 	Swiss French
    0x0810 	Swiss Italian 		0x0816 	Portuguese (Portugal)
    0x0813 	Belgian Dutch 		0x081A 	Serbo-Croatian (Cyrillic)
    0x0814 	Norwegian � Nynorsk 	  	 

charsetID
    One of the following character-set identifiers.

    Identifier 	Character Set

    0 		7-bit ASCII
    932 	Japan (Shift %G�%@ JIS X-0208)
    949 	Korea (Shift %G�%@ KSC 5601)
    950 	Taiwan (Big5)
    1200 	Unicode
    1250 	Latin-2 (Eastern European)
    1251 	Cyrillic
    1252 	Multilingual
    1253 	Greek
    1254 	Turkish
    1255 	Hebrew
    1256 	Arabic

string-name
    One of the following predefined names.

    Name 		Description

    Comments 		Additional information that should be displayed for
			diagnostic purposes.
    CompanyName 	Company that produced the file%G�%@for example,
			"Microsoft Corporation" or "Standard Microsystems
			Corporation, Inc." This string is required.
    FileDescription 	File description to be presented to users. This
			string may be displayed in a list box when the user
			is choosing files to install%G�%@for example,
			"Keyboard Driver for AT-Style Keyboards". This
			string is required.
    FileVersion 	Version number of the file%G�%@for example,
			"3.10" or "5.00.RC2". This string is required.
    InternalName 	Internal name of the file, if one exists � for
			example, a module name if the file is a dynamic-link
			library. If the file has no internal name, this
			string should be the original filename, without
			extension. This string is required.
    LegalCopyright 	Copyright notices that apply to the file. This
			should include the full text of all notices, legal
			symbols, copyright dates, and so on � for example,
			"Copyright (C) Microsoft Corporation 1990�1999".
			This string is optional.
    LegalTrademarks 	Trademarks and registered trademarks that apply to
			the file. This should include the full text of all
			notices, legal symbols, trademark numbers, and so on.
			This string is optional.
    OriginalFilename 	Original name of the file, not including a path.
			This information enables an application to determine
			whether a file has been renamed by a user. The
			format of the name depends on the file system for
			which the file was created. This string is required.
    PrivateBuild 	Information about a private version of the file � for
			example, "Built by TESTER1 on \TESTBED". This string
			should be present only if VS_FF_PRIVATEBUILD is
			specified in the fileflags parameter of the root
			block.
    ProductName 	Name of the product with which the file is
			distributed. This string is required.
    ProductVersion 	Version of the product with which the file is
			distributed � for example, "3.10" or "5.00.RC2".
			This string is required.
    SpecialBuild 	Text that indicates how this version of the file
			differs from the standard version � for example,
			"Private build for TESTER1 solving mouse problems
			on M250 and M250E computers". This string should be
			present only if VS_FF_SPECIALBUILD is specified in
			the fileflags parameter of the root block.
 */
