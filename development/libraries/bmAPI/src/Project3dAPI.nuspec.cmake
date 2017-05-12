<?xml version="1.0"?>
<package >
  <metadata>
    <id>@CSPROJ_NAMESPACE@.@CSPROJ_NAME@</id>
    <version>@SPEC_FILE_GIT_DATE_AS_VER@</version>
    <authors>Sergey Koshelev</authors>
    <owners>Sergey Koshelev</owners>
    <projectUrl>https://sede-pt-ssw.visualstudio.com/DefaultCollection/Basin-Evaluation/_git/Basin-Modelling</projectUrl>
    <iconUrl>file://@CSPROJ_NAME@.png</iconUrl>
    <requireLicenseAcceptance>false</requireLicenseAcceptance>
    <description>C# API for Cauldron Project3d reading/writing. @CMAKE_BUILD_TYPE@ buid</description>
    <releaseNotes>2017.105.101 Initial API release </releaseNotes>
    <copyright>Copyright 2017</copyright>
    <tags>Cauldron backend Project3d API</tags>
    <references>
      <reference file="@CSPROJ_NAMESPACE@.@CSPROJ_NAME@.dll" />
    </references>
    <dependencies>
      <!-- <dependency id="SampleDependency" version="1.0" /> -->
    </dependencies>
  </metadata>
  <files>  
    <!-- Swig C# assembly -->
    <file src="libraries\bmAPI\csharp\Release\@CSPROJ_NAMESPACE@.@CSPROJ_NAME@.dll" target="lib\net@DONET_TARGET_VERSION_SHORT@\Release" />
    <file src="libraries\bmAPI\csharp\Debug\@CSPROJ_NAMESPACE@.@CSPROJ_NAME@.dll"   target="lib\net@DONET_TARGET_VERSION_SHORT@\Debug" />
    <file src="libraries\bmAPI\csharp\Debug\*.pdb"   target="lib\net@DONET_TARGET_VERSION_SHORT@\Debug" />
    <!-- Swig C++ native library -->
    <file src="libraries\bmAPI\Release\@CSPROJ_NAME@.dll" target="lib\native\x64\Release" />
    <file src="libraries\bmAPI\Debug\@CSPROJ_NAME@.dll" target="lib\native\x64\Debug" />
    <file src="libraries\bmAPI\Debug\@CSPROJ_NAME@.pdb" target="lib\native\x64\Debug" />
    <!-- Modificator for destination c# project to copy native dll -->
    <file src="@CMAKE_BINARY_DIR@\@CSPROJ_NAMESPACE@.@CSPROJ_NAME@.targets" target="build" />
  </files>
</package>