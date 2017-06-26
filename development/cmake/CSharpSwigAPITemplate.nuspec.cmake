<?xml version="1.0"?>
<package >
  <metadata>
    <id>@CSPROJ_NAMESPACE@.@CSPROJ_NAME@</id>
    <version>@CSPROJ_ASSEMBLY_VERSION@</version>
    <authors>@CSPROJ_ASSEMBLY_OWNER@</authors>
    <owners>@CSPROJ_ASSEMBLY_OWNER@</owners>
    <projectUrl>@NUGET_VSTS_FEED_URL@</projectUrl>
    <iconUrl>file://@CSPROJ_NAME@.png</iconUrl>
    <requireLicenseAcceptance>false</requireLicenseAcceptance>
    <description>@CSPROJ_ASSEMBLY_DESCRIPTION@</description>
    <releaseNotes>@CSPROJ_ASSEMBLY_RELEASE_NOTES@</releaseNotes>
    <copyright>@CSPROJ_ASSEMBLY_COPYRIGHT@</copyright>
    <tags>@CSPROJ_ASSEMBLY_TAGS@</tags>
    <references>
      <reference file="@CSPROJ_NAMESPACE@.@CSPROJ_NAME@.dll" />
    </references>
    <dependencies>
      <!-- <dependency id="SampleDependency" version="1.0" /> -->
    </dependencies>
  </metadata>
  <files>  
    <!-- Swig C# assembly -->
    <file src="libraries\@PROJ_NAME@\csharp\Release\@CSPROJ_NAMESPACE@.@CSPROJ_NAME@.dll" target="lib\net@DONET_TARGET_VERSION_SHORT@\Release" />
    <file src="libraries\@PROJ_NAME@\csharp\Debug\@CSPROJ_NAMESPACE@.@CSPROJ_NAME@.dll"   target="lib\net@DONET_TARGET_VERSION_SHORT@\Debug" />
    <file src="libraries\@PROJ_NAME@\csharp\Debug\*.pdb"   target="lib\net@DONET_TARGET_VERSION_SHORT@\Debug" />
    <!-- Swig C++ native library -->
    <file src="libraries\@PROJ_NAME@\Release\@CSPROJ_NAME@.dll" target="lib\native\x64\Release" />
    <file src="libraries\@PROJ_NAME@\Debug\@CSPROJ_NAME@.dll" target="lib\native\x64\Debug" />
    <file src="libraries\@PROJ_NAME@\Debug\@CSPROJ_NAME@.pdb" target="lib\native\x64\Debug" />
    <file src="c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\debug_nonredist\x64\Microsoft.VC140.DebugCRT\msvcp140d.dll" target="lib\native\x64\Debug" />
    <file src="c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\debug_nonredist\x64\Microsoft.VC140.DebugCRT\vcruntime140d.dll" target="lib\native\x64\Debug" />
    <!-- Modificator for destination c# project to copy native dll -->
    <file src="@CMAKE_BINARY_DIR@\@CSPROJ_NAMESPACE@.@CSPROJ_NAME@.targets" target="build" />
  </files>
</package>