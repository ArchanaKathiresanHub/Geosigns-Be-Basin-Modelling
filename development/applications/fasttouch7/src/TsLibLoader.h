#ifndef TSLIBLOADER_H 
#define TSLIBLOADER_H

#include <vector>

//-- Touchstone Library Header Files --//
//-- =============================== --//

#ifdef __INTEL_COMPILER // disable warning "type qualifier on return type is meaningless" in tslibI.h
#pragma warning push
#pragma warning(disable:858)
#endif

#include <TsLibf.h>

#ifdef __INTEL_COMPILER
#pragma warning pop
#endif

#include <libloader.h>

using namespace std;

namespace Geocosm 
{
    #ifndef TSLIB_EXPORTS
    #define TsLibPluginManager sTsLibPluginManager
    #define TsLibLoadedPlugin sTsLibLoadedPlugin
    #endif


    /** A class to handle Touchstone library plugins. */
    class TsLibLoadedPlugin 
    {
        public:
            
            //-- Public members            --//
            //-- ==============            --//
        
            /** Library handle. */
            LIB_HANDLE libHandle;
            std::string pluginDllFile;
            CREATE_TSLIB_PROC createTsLibFuncAdder;		
            FINALIZE_TSLIB_PROC finializeFuncAdder;
            INITIALIZE_TSLIB_PROC initializeFuncAdder;
            CREATE_TCFINFO_PROC createTcfInfoFuncAdder;
    };
    
    
    /** A class to manage Touchstone library plugins. */
    class TsLibPluginManager
    {
    
        public:
            static void LoadPluginLibraries(const std::string& startupDirectory);
            static void UnloadPluginLibraries();
            static Geocosm::TsLib::TsLibInterface* CreateTsLibPlugin();
            static Geocosm::TsLib::TcfInfoInterface* CreateTcfInfo(); 
    
        private:
            static std::vector<TsLibLoadedPlugin> m_pluginsLibs;
            static std::string m_startupDirectory;
            static bool m_initialized;
    };
}
#endif
