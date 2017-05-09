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
    <RootNamespace>Shell.BasinModeling.Cauldron</RootNamespace>
    <AssemblyName>Shell.BasinModeling.Cauldron</AssemblyName>
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
     <Delete Files="obj/$(Platform)/$(Configuration)/Shell.BasinModeling.Cauldron.dll" Condition="Exists('obj/$(Platform)/$(Configuration)/Shell.BasinModeling.Cauldron.dll')" />
	 <Delete Files="$(Configuration)/Shell.BasinModeling.Cauldron.dll" Condition="Exists('$(Configuration)/Shell.BasinModeling.Cauldron.dll')" />
  </Target>
  <Target Name="AfterBuild">
     <Error Condition="'@(Compile)' == ''" Text="No source files included in build of Shell.BasinModeling.Cauldron.dll. Reload the project, and build the solution again"/>
  </Target>
</Project>