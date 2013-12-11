#ifndef TSLIBLOADER_H
#define TSLIBLOADER_H

#include <TsLibf.h>
#include <vector>
#include <libloader.h>
using namespace std;

namespace Geocosm {

#ifndef TSLIB_EXPORTS
#define TsLibPluginManager sTsLibPluginManager
#define TsLibLoadedPlugin sTsLibLoadedPlugin
#endif

	class TsLibLoadedPlugin {
	public:
		LIB_HANDLE libHandle;		
		std::string pluginDllFile;
		CREATE_TSLIB_PROC createTsLibFuncAdder;		
		FINALIZE_TSLIB_PROC finializeFuncAdder;
		INITIALIZE_TSLIB_PROC initializeFuncAdder;
		CREATE_TCFINFO_PROC createTcfInfoFuncAdder;
	};

	class TsLibPluginManager {
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
