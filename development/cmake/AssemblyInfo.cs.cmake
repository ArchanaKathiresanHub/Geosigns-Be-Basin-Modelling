using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

// General Information about an assembly is controlled through the following 
// set of attributes. Change these attribute values to modify the information
// associated with an assembly.
[assembly: AssemblyTitle("@CSPROJ_NAMESPACE@.@CSPROJ_NAME@.Properties")]
[assembly: AssemblyDescription("@CSPROJ_ASSEMBLY_DESCRIPTION@")]
#if DEBUG
[assembly: AssemblyConfiguration("Debug")]
#else
[assembly: AssemblyConfiguration("Release")]
#endif
[assembly: AssemblyCompany("PDS B.V.")]
[assembly: AssemblyProduct("@BM_VERSION_STRING@ Build @BM_BUILD_NUMBER@")]
[assembly: AssemblyCopyright("Copyright © Shell 2015-2017")]
[assembly: AssemblyTrademark("")]
[assembly: AssemblyCulture("")]

// Setting ComVisible to false makes the types in this assembly not visible 
// to COM components.  If you need to access a type in this assembly from 
// COM, set the ComVisible attribute to true on that type.
[assembly: ComVisible(false)]

// The following GUID is for the ID of the typelib if this project is exposed to COM
[assembly: Guid("3babc17c-0db1-4a0d-a305-3661409f055e")]
[assembly: AssemblyVersion("@CSPROJ_ASSEMBLY_VERSION@")]
[assembly: AssemblyFileVersion("@CSPROJ_ASSEMBLY_VERSION@")]
