<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup>
    <Configuration Condition=" '$(Configuration)' == '' ">Debug</Configuration>
    <Platform Condition=" '$(Platform)' == '' ">@BM_WINDOWS_PLATFORM@</Platform>
    <ProductVersion>8.0.30703</ProductVersion>
    <SchemaVersion>2.0</SchemaVersion>
    <ProjectGuid>{@Guid@}</ProjectGuid>
    <OutputType>Library</OutputType>
    <AppDesignerFolder>Properties</AppDesignerFolder>
    <RootNamespace>@CSPROJ_NAMESPACE@.@CSPROJ_NAME@</RootNamespace>
    <AssemblyName>@CSPROJ_NAMESPACE@.@CSPROJ_NAME@</AssemblyName>
    <TargetFrameworkVersion>v@DONET_TARGET_VERSION@</TargetFrameworkVersion>
    <FileAlignment>512</FileAlignment>
  </PropertyGroup>
  <PropertyGroup Condition=" '$(Configuration)|$(Platform)' == 'Debug|@BM_WINDOWS_PLATFORM@' ">
    <DebugSymbols>true</DebugSymbols>
    <DebugType>full</DebugType>
    <Optimize>false</Optimize>
    <OutputPath>Debug\</OutputPath>
    <DefineConstants>DEBUG;TRACE</DefineConstants>
    <ErrorReport>prompt</ErrorReport>
    <WarningLevel>4</WarningLevel>
    <PlatformTarget>AnyCPU</PlatformTarget>
  </PropertyGroup>
  <PropertyGroup Condition=" '$(Configuration)|$(Platform)' == 'Release|@BM_WINDOWS_PLATFORM@' ">
    <DebugType>pdbonly</DebugType>
    <Optimize>true</Optimize>
    <OutputPath>Release\</OutputPath>
    <DefineConstants>TRACE</DefineConstants>
    <ErrorReport>prompt</ErrorReport>
    <WarningLevel>4</WarningLevel>
  </PropertyGroup>
  <ItemGroup>
    <Reference Include="System" />
    <Reference Include="System.Core" />
    <Reference Include="System.Xml.Linq" />
    <Reference Include="System.Data.DataSetExtensions" />
    <Reference Include="System.Data" />
    <Reference Include="System.Xml" />
  </ItemGroup>
  <ItemGroup>
    <Compile Include="*.cs" />
  </ItemGroup>
    
  <!-- Include C# targets, because this is a C# project -->
  <Import Project="$(MSBuildToolsPath)\Microsoft.CSharp.targets" />
  
  <Target Name="BeforeBuild">
     <!-- Because Visual studio doesn't handle wildcards very well, a rebuild is forced each time by removing the resulting files -->
     <Delete Files="obj/$(Platform)/$(Configuration)/Shell.BasinModeling.@CSPROJ_NAME@.dll" Condition="Exists('obj/$(Platform)/$(Configuration)/Shell.BasinModeling.@CSPROJ_NAME@.dll')" />
	 <Delete Files="$(Configuration)/Shell.BasinModeling.@CSPROJ_NAME@.dll" Condition="Exists('$(Configuration)/Shell.BasinModeling.@CSPROJ_NAME@.dll')" />
  </Target>
  <Target Name="AfterBuild">
     <Error Condition="'@(Compile)' == ''" Text="No source files included in build of Shell.BasinModeling.@CSPROJ_NAME@.dll. Reload the project, and build the solution again"/>
  </Target>
  <PropertyGroup Condition=" '$(Configuration)' == 'Release' ">
    <PostBuildEvent>cd @CMAKE_CURRENT_BINARY_DIR@/../../</PostBuildEvent>
	
	<!-- The pack option is disable here because the Project3dAPI NuGet creation is performed separately -->
	<!-- Removal of these lines requires further investigation -->
	<!-- <PostBuildEvent>cd @CMAKE_CURRENT_BINARY_DIR@/../../ -->
    <!-- #@NUGET_EXECUTABLE@ pack @CSPROJ_NAME@.nuspec</PostBuildEvent> -->
  </PropertyGroup>
</Project>