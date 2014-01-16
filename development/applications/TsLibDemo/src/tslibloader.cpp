#include "tslibloader.h"
#include <geocosmexception.h>
#include <libloader.h>
using namespace std;

#ifdef _WIN32
#define TSLIB_DIR_SEPARATOR "\\"
#define TSLIB_DIR_SEPARATOR_CHAR '\\'
#define TSLIB_LIBRARY_EXTS  "*.dll"
#define PLUGINDIR "plugins\\"
#define TSLIB_NAME "tslib.dll"
#else
#define TSLIB_DIR_SEPARATOR "/"
#define TSLIB_DIR_SEPARATOR_CHAR '/'
#define TSLIB_LIBRARY_EXTS "*.so"
#define TSLIB_NAME "libtslib.so"
#endif

namespace Geocosm {

   std::string itoa( const int & x )
   {
      std::ostringstream o;
      if ( !(o << x) ) return "ERROR";
      return o.str();
   }

#ifdef _WIN32
   std::string GetLastDLLError()
   {
      LPVOID lpMsgBuf = NULL;
      DWORD dw = GetLastError(); 

      FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                     NULL,
                     dw,
                     MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                     (LPTSTR) &lpMsgBuf,
                     0, NULL );
      std::string retMsg;
      if ( lpMsgBuf != NULL )
      {
         retMsg = std::string("Windows error " + itoa(dw) + std::string(" ") + std::string((const char*)lpMsgBuf));
         LocalFree(lpMsgBuf);
      }
      else
      {
         retMsg = std::string("Windows error " + itoa(dw));
      }
      return retMsg;
   }
#else
   std::string GetLastDLLError()
   {
      char * errstr = dlerror();
      if ( errstr != NULL )
      {
         return std::string( errstr );
      }
      return std::string( "Unknown library load error" );
   }
#endif

   // TsLibPluginManager
   bool TsLibPluginManager::m_initialized = false;
   std::string TsLibPluginManager::m_startupDirectory = std::string();
   std::vector<TsLibLoadedPlugin> TsLibPluginManager::m_pluginsLibs = std::vector<TsLibLoadedPlugin>();

   void TsLibPluginManager::LoadPluginLibraries( const std::string & startupDirectory )
   {
      m_startupDirectory = std::string(startupDirectory);

      if ( m_startupDirectory[m_startupDirectory.size() - 1] != TSLIB_DIR_SEPARATOR_CHAR )
      {
         m_startupDirectory.append( TSLIB_DIR_SEPARATOR );
      }

      std::string filestr( m_startupDirectory );
      filestr.append( TSLIB_NAME );

      m_pluginsLibs.clear();
      Geocosm::TsLibLoadedPlugin currentPlugin;

      LIB_HANDLE libHandle = Geocosm::LibLoader::LoadSharedLibrary( filestr.c_str() );
      if ( libHandle == NULL )
      {
         throw GeocosmException(std::string("Error loading tslib plug-in: ") + filestr,GetLastDLLError());
      }

      currentPlugin.createTsLibFuncAdder = (CREATE_TSLIB_PROC)LibLoader::GetFunction( libHandle, (const char*)"CreateTsLibInterface" );
      if ( currentPlugin.createTsLibFuncAdder == NULL )
      {
         //not a dll we are interested in
         LibLoader::FreeSharedLibrary(libHandle);
         throw GeocosmException(std::string("CreateTsLibInterface function not found in plug-in: ") + filestr);             
      }
      currentPlugin.initializeFuncAdder = (INITIALIZE_TSLIB_PROC)LibLoader::GetFunction( libHandle, (const char*)"InitializeTsLibInterface");
      if ( currentPlugin.initializeFuncAdder == NULL )
      {
         //not a dll we are interested in
         LibLoader::FreeSharedLibrary(libHandle);
         throw GeocosmException(std::string("InitializeTsLibInterface function not found in plug-in: ") + filestr);             
      }
      currentPlugin.finializeFuncAdder = (FINALIZE_TSLIB_PROC)LibLoader::GetFunction( libHandle, (const char*)"FinalizeTsLibInterface");
      if ( currentPlugin.finializeFuncAdder == NULL )
      {
         //not a dll we are interested in
         LibLoader::FreeSharedLibrary(libHandle);
         throw GeocosmException(std::string("FinalizeTsLibInterface function not found in plug-in: ") + filestr);             
      }
      currentPlugin.createTcfInfoFuncAdder = (CREATE_TCFINFO_PROC)LibLoader::GetFunction( libHandle, (const char*)"CreateTcfInfoInterface");
      if ( currentPlugin.createTcfInfoFuncAdder == NULL )
      {
         //not a dll we are interested in
         LibLoader::FreeSharedLibrary(libHandle);
         throw GeocosmException(std::string("CreateTcfInfoInterface function not found in plug-in: ") + filestr);             
      }
      unsigned int err = currentPlugin.initializeFuncAdder(m_startupDirectory.c_str());
      if (err != 0)
      {
         LibLoader::FreeSharedLibrary(libHandle);
         throw GeocosmException(std::string("InitializeTsLibInterface function returned error from plug-in: ") + filestr,itoa(err));             
      }
      currentPlugin.pluginDllFile = filestr;            
      currentPlugin.libHandle = libHandle;

      m_pluginsLibs.push_back(currentPlugin);         

      //make sure we have at least one tslib 
      if ( m_pluginsLibs.size() == 0 )
      {
         throw GeocosmException(std::string("TsLib plug-in not found."),std::string("Unable to proceed."));
      }
      m_initialized = true;
   }

   void TsLibPluginManager::UnloadPluginLibraries()
   {
      for ( std::vector<Geocosm::TsLibLoadedPlugin>::iterator itor = m_pluginsLibs.begin(); itor != m_pluginsLibs.end(); itor++ )
      {
         if ( (*itor).libHandle != NULL )
         {
            (*itor).finializeFuncAdder();
            LibLoader::FreeSharedLibrary( (*itor).libHandle );
            (*itor).libHandle = NULL;
         }
      }
      m_pluginsLibs.clear();
      m_initialized = false;
   }

   Geocosm::TsLib::TsLibInterface* TsLibPluginManager::CreateTsLibPlugin()
   {
      for ( std::vector<Geocosm::TsLibLoadedPlugin>::iterator itor = m_pluginsLibs.begin(); itor != m_pluginsLibs.end(); itor++ )
      {
         //just use the first one, as there should only be one...
         Geocosm::TsLib::TsLibInterface * pluginTsLib = NULL;
         unsigned int err = (*itor).createTsLibFuncAdder( &pluginTsLib );
         if ( (pluginTsLib == NULL)  || (err != 0) )
         {
            throw GeocosmException(std::string("CreateTsLibInterface function returned error from plug-in: ") + (*itor).pluginDllFile,itoa(err));
         }
         return pluginTsLib;
      }
      throw GeocosmException(std::string("Unable to find loaded plug-in for Tslib"),"TsLibPluginManager::CreateTsLibPlugin");
   }
   
   Geocosm::TsLib::TcfInfoInterface* TsLibPluginManager::CreateTcfInfo()
   {
      for ( std::vector<Geocosm::TsLibLoadedPlugin>::iterator itor = m_pluginsLibs.begin(); itor != m_pluginsLibs.end(); itor++ )
      {
         //just use the first one, as there should only be one...
         Geocosm::TsLib::TcfInfoInterface * tcfInfo = NULL;
         unsigned int err = (*itor).createTcfInfoFuncAdder( &tcfInfo );
         if ( (tcfInfo == NULL)  || (err != 0) )
         {
            throw GeocosmException( std::string( "CreateTcfInfoFuncAdder function returned error from plug-in: " ) + (*itor).pluginDllFile, itoa(err) );
         }
         return tcfInfo;
      }
      throw GeocosmException(std::string( "Unable to find loaded plug-in for Tslib" ),"TsLibPluginManager::CreateTcfInfo" );
   }
}
