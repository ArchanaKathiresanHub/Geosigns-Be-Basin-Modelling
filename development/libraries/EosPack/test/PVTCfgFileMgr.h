#ifndef PVT_CFG_FILE_MGR
#define PVT_CFG_FILE_MGR


/// @brief Axillary class for set of unit tests. It creates config file for PVT library
/// in current folder in constructor and delete it in destructor. Also it set up cfg file
/// name for EosPack 
class PVTCfgFileMgr
{
public:
   ~PVTCfgFileMgr();

   static const PVTCfgFileMgr & instance( const char * nm = 0 ); ///!< object factory

private:
   PVTCfgFileMgr( const char * nm );
   const char *        m_cfgFileName; ///!< Name of the cfg file (optional)
   static const char * s_cfgFileName; ///!< Name of the cfg file
};

#endif
