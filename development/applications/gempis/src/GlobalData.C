///
///
///

#include "includestreams.h"

#include <GlobalData.h>


QString GlobalData::s_path = "";

void GlobalData::SetPath( const QString & path )
{
#ifdef DEBUG_GLOBALDATA
  cout << " GlobalData::GlobalData()" << endl;
#endif

  s_path = path;
}


const QString & GlobalData::GetPath( void )
{
#ifdef DEBUG_GLOBALDATA
  cout << " GlobalData::GetPath()" << endl;
#endif

  return s_path;
}















