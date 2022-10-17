:: This .bat script will create the Project3dAPI NuGet package automatically
:: Default location to keep this file is ${Build_Location}/libraries/Project3dAPI
:: If the file is kept at other location, then before line 8, cd to ${Build_Location}/libraries/Project3dAPI

@echo off

:: Creating the directory to build the nuget package
mkdir Project3dAPINuGetPackage
cd Project3dAPINuGetPackage

:: Creating the required directory structure
mkdir build
mkdir lib

mkdir lib\native
mkdir lib\native\x64

mkdir lib\native\x64\Debug
copy "..\Debug\Project3dAPI.dll" lib\native\x64\Debug
copy "..\Debug\Project3dAPI.pdb" lib\native\x64\Debug

mkdir lib\native\x64\Release
copy "..\Release\Project3dAPI.dll" lib\native\x64\Release

mkdir lib\net48
mkdir lib\net48\Debug
copy "..\csharp\Debug\Bpa2.Basin.Project3d.Project3dAPI.dll" lib\net48\Debug
copy "..\csharp\Debug\Bpa2.Basin.Project3d.Project3dAPI.pdb" lib\net48\Debug

mkdir lib\net48\Release
copy "..\csharp\Release\Bpa2.Basin.Project3d.Project3dAPI.dll" lib\net48\Release

echo "All required files are successfully copied"



:: -----------------------------------------------------------------------------------------------------------------
:: Generating the nuget.config file
set "TAB=    "

(
	echo ^<^?xml version=^"1.0^" encoding=^"utf-8^"^?^>
	echo ^<configuration^>
	echo		^<packageSources^>
	echo			^<clear /^>
	echo			^<add key=^"BPA2^" value=^"https://sede-pt-ssw.pkgs.visualstudio.com/_packaging/BPA2/nuget/v3/index.json^" /^>
	echo		^</packageSources^>
	echo ^</configuration^>
) > nuget.config
echo "nuget.config file is created successfully"

:: -----------------------------------------------------------------------------------------------------------------



:: -----------------------------------------------------------------------------------------------------------------
:: Creating the Bpa2.Basin.Project3d.Project3dAPI.nuspec file
set Version=%date:~10,4%%date:~4,2%.%date:~7,2%.%time:~0,2%.%time:~3,2%

(
	echo ^<^?xml version=^"1.0^" encoding=^"utf-8^"^?^>
	echo ^<package xmlns=^"http://schemas.microsoft.com/packaging/2013/05/nuspec.xsd^"^>
	echo  	^<metadata^>
	echo    		^<id^>Bpa2.Basin.Project3d.Project3dAPI^</id^>
	echo    		^<version^>%Version%^</version^>
	echo    		^<authors^>ADO Pipeline^</authors^>
	echo    		^<owners^>Shell International^</owners^>
	echo    		^<requireLicenseAcceptance^>false^</requireLicenseAcceptance^>
	echo    		^<projectUrl^>https://sede-pt-ssw.pkgs.visualstudio.com/_packaging/BPA2/nuget/v3/index.json^</projectUrl^>
	echo    		^<description^>C# API for Cauldron Project3d file reading/writing.^</description^>
	echo    		^<releaseNotes^>Version %Version%. Updated nuget package release^</releaseNotes^>
	echo    		^<copyright^>"Copyright (C) 2017 - 2022"^</copyright^>
	echo    		^<tags^>Cauldron backend Project3d API^</tags^>
	echo    		^<dependencies^>
	echo      		^<dependency id="Bpa2.Basin.VS2019CppRedistDebugDlls" version="(, 2.0.0)" /^>
	echo    		^</dependencies^>
	echo    		^<references^>
	echo      		^<reference file=^"Bpa2.Basin.Project3d.Project3dAPI.dll^" /^>
	echo    		^</references^>
	echo  	^</metadata^>
	echo ^</package^>
) > Bpa2.Basin.Project3d.Project3dAPI.nuspec 
echo "Bpa2.Basin.Project3d.Project3dAPI.nuspec file is created successfully"

:: -----------------------------------------------------------------------------------------------------------------


:: -----------------------------------------------------------------------------------------------------------------
:: Creating the Bpa2.Basin.Project3d.Project3dAPI.targets file within the build folder
cd build
(
	echo ^<^?xml version=^"1.0^" encoding=^"utf-8^"^?^>
	echo ^<Project ToolsVersion=^"4.0^" xmlns=^"http://schemas.microsoft.com/developer/msbuild/2003^"^>
	echo 	^<ItemGroup Condition=" '$(Configuration)' == '' Or '$(Configuration)' == 'Release'"^>
	echo 		^<ReferenceCopyLocalPaths 
	echo 			Include="$(MSBuildThisFileDirectory)..\lib\native\x64\Release\Project3dAPI.dll" 
	echo 		/^>
	echo 		^<ReferenceCopyLocalPaths 
    echo 			Include="$(MSBuildThisFileDirectory)..\lib\net48\Release\Bpa2.Basin.Project3d.Project3dAPI.dll" 
    echo 		/^>
	echo 	^</ItemGroup^>
	echo 	^<ItemGroup Condition=" '$(Configuration)' == 'Debug' "^>
	echo 		^<ReferenceCopyLocalPaths       
	echo 			Include="$(MSBuildThisFileDirectory)..\lib\native\x64\Debug\Project3dAPI.dll" 
    echo 		/^>
    echo 		^<ReferenceCopyLocalPaths
    echo 			Include="$(MSBuildThisFileDirectory)..\lib\native\x64\Debug\Project3dAPI.pdb" 
    echo 		/^>
    echo 		^<ReferenceCopyLocalPaths 
    echo 			Include="$(MSBuildThisFileDirectory)..\lib\net48\Debug\Bpa2.Basin.Project3d.Project3dAPI.pdb" 
    echo 		/^>
    echo 		^<ReferenceCopyLocalPaths 
    echo 			Include="$(MSBuildThisFileDirectory)..\lib\net48\Debug\Bpa2.Basin.Project3d.Project3dAPI.dll" 
    echo 		/^>
	echo 	^</ItemGroup^>
	echo ^</Project^>
) > Bpa2.Basin.Project3d.Project3dAPI.targets
echo "Bpa2.Basin.Project3d.Project3dAPI.targets file is created successfully"

:: -----------------------------------------------------------------------------------------------------------------

cd ..
:: Creating the nuget package
nuget pack -Prop Configuration=Release -IncludeReferencedProjects -Exclude ".vs"


cd ..