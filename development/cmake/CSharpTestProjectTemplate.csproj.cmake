﻿<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="12.0" DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup>
    <Configuration Condition=" '$(Configuration)' == '' ">Debug</Configuration>
    <Platform Condition=" '$(Platform)' == '' ">@Platform@</Platform>


    <ProjectGuid>{@Guid@}</ProjectGuid>
    <OutputType>Library</OutputType>
    <AppDesignerFolder>Properties</AppDesignerFolder>
    <RootNamespace>@TestProjectName@</RootNamespace>
    <AssemblyName>@TestProjectName@</AssemblyName>
    <TargetFrameworkVersion>v@DONET_TARGET_VERSION@</TargetFrameworkVersion>
    <FileAlignment>512</FileAlignment>
    <ProjectTypeGuids>{3AC096D0-A1C2-E12C-1390-A8335801FDAB};{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}</ProjectTypeGuids>

    <VisualStudioVersion Condition="'$(VisualStudioVersion)' == ''">10.0</VisualStudioVersion>
    <VSToolsPath Condition="'$(VSToolsPath)' == ''">$(MSBuildExtensionsPath32)\Microsoft\VisualStudio\v$(VisualStudioVersion)</VSToolsPath>
    <ReferencePath>$(ProgramFiles)\Common Files\microsoft shared\VSTT\$(VisualStudioVersion)\UITestExtensionPackages</ReferencePath>
    <IsCodedUITest>False</IsCodedUITest>
    <TestProjectType>UnitTest</TestProjectType>
  </PropertyGroup>
  <PropertyGroup Condition=" '$(Configuration)|$(Platform)' == 'Debug|@Platform@' ">
    <DebugSymbols>true</DebugSymbols>
    <DebugType>full</DebugType>
    <Optimize>false</Optimize>
    <OutputPath>bin\Debug\</OutputPath>
    <DefineConstants>DEBUG;TRACE</DefineConstants>
    <ErrorReport>prompt</ErrorReport>
    <WarningLevel>4</WarningLevel>
    <PlatformTarget>AnyCPU</PlatformTarget>
  </PropertyGroup>
  <PropertyGroup Condition=" '$(Configuration)|$(Platform)' == 'Release|@Platform@' ">
    <DebugType>pdbonly</DebugType>
    <Optimize>true</Optimize>
    <OutputPath>bin\Release\</OutputPath>
    <DefineConstants>TRACE</DefineConstants>
    <ErrorReport>prompt</ErrorReport>
    <WarningLevel>4</WarningLevel>
    <PlatformTarget>AnyCPU</PlatformTarget>
  </PropertyGroup>
  <ItemGroup>

    <Reference Include="System" />
  </ItemGroup>
  <Choose>
    <When Condition="('$(VisualStudioVersion)' == '10.0' or '$(VisualStudioVersion)' == '') and '$(TargetFrameworkVersion)' == 'v3.5'">
      <ItemGroup>
        <Reference Include="Microsoft.VisualStudio.QualityTools.UnitTestFramework, Version=10.1.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a, processorArchitecture=MSIL" />
      </ItemGroup>
    </When>
    <Otherwise>
      <ItemGroup>
        <Reference Include="Microsoft.VisualStudio.QualityTools.UnitTestFramework" />

      </ItemGroup>
    </Otherwise>
  </Choose>
  <ItemGroup>
    <Compile Include="Properties\AssemblyInfo.cs" />
    <Compile Include="@FileListForCompilation@" />
  </ItemGroup>
  <ItemGroup>
	@ProjectReferences@
  </ItemGroup>
  <ItemGroup>
	@DeploymentItemsList@
  </ItemGroup>
  <Choose>
    <When Condition="'$(VisualStudioVersion)' == '10.0' And '$(IsCodedUITest)' == 'True'">
      <ItemGroup>
        <Reference Include="Microsoft.VisualStudio.QualityTools.CodedUITestFramework, Version=10.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a, processorArchitecture=MSIL">
          <Private>False</Private>
        </Reference>
        <Reference Include="Microsoft.VisualStudio.TestTools.UITest.Common, Version=10.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a, processorArchitecture=MSIL">
          <Private>False</Private>
        </Reference>
        <Reference Include="Microsoft.VisualStudio.TestTools.UITest.Extension, Version=10.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a, processorArchitecture=MSIL">
          <Private>False</Private>
        </Reference>
        <Reference Include="Microsoft.VisualStudio.TestTools.UITesting, Version=10.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a, processorArchitecture=MSIL">
          <Private>False</Private>
        </Reference>
      </ItemGroup>
    </When>
  </Choose>
  <Import Project="$(VSToolsPath)\TeamTest\Microsoft.TestTools.targets" Condition="Exists('$(VSToolsPath)\TeamTest\Microsoft.TestTools.targets')" />
  <Import Project="$(MSBuildToolsPath)\Microsoft.CSharp.targets" />
  <!-- To modify your build process, add your task inside one of the targets below and uncomment it. 
       Other similar extension points exist, see Microsoft.Common.targets.
  <Target Name="BeforeBuild">
  </Target>
  <Target Name="AfterBuild">
  </Target>
  -->
</Project>
