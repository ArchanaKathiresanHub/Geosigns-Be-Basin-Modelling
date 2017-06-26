<?xml version="1.0" encoding="utf-8"?> 
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003"> 
    <ItemGroup Condition=" '$(Configuration)' == '' Or '$(Configuration)' == 'Release' ">
      <ReferenceCopyLocalPaths Include="$(MSBuildThisFileDirectory)..\lib\native\x64\Release\@CSPROJ_NAME@.dll" />
    </ItemGroup> 
    <ItemGroup Condition=" '$(Configuration)' == 'Debug' ">
      <ReferenceCopyLocalPaths Include="$(MSBuildThisFileDirectory)..\lib\native\x64\Debug\@CSPROJ_NAME@.dll" />
      <ReferenceCopyLocalPaths Include="$(MSBuildThisFileDirectory)..\lib\native\x64\Debug\@CSPROJ_NAME@.pdb" />
      <ReferenceCopyLocalPaths Include="$(MSBuildThisFileDirectory)..\lib\net452\Debug\@CSPROJ_NAMESPACE@.@CSPROJ_NAME@.pdb" />
    </ItemGroup> 
 </Project>
   