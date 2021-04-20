#include "anonymizer.h"
#include <iostream>
#include "boost/program_options.hpp"

#ifdef _WIN32
#include "ShObjIdl.h"
#include "shlobj.h"
#include <tchar.h>

namespace guiWindow
{
  std::string getFolder()
  {
    std::string out;
    BROWSEINFO bi = {0};
    bi.lpszTitle = _T("Select project folder to be anonymized");
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if(pidl != 0)
    {
        // get the name of the folder
        TCHAR path[MAX_PATH];
        SHGetPathFromIDList(pidl, path);
        out = path;
    
        // free memory used
        IMalloc * imalloc = 0;
        if(SUCCEEDED(SHGetMalloc(&imalloc)))
        {
            imalloc->Free(pidl);
            imalloc->Release();
        }
    }
    return out;
  }
}
#endif

int main( int argc, char *argv[] )
{
   boost::program_options::options_description desc("Available options");
   desc.add_options()
      ("help", "show help message")
      ("projectFolder", boost::program_options::value<std::string>(), "Path of project folder to be anonymized")
      ("shiftCoord", boost::program_options::value<bool>()->default_value(true), "Enables coordinates shift")
#ifndef _WIN32
      ("clear2DAttributes", boost::program_options::value<std::string>(), "Path of the HDF file (removes GridName, StratTopName and sets origin to 0)")
#endif
      ;

   boost::program_options::variables_map vm;
   boost::program_options::store( boost::program_options::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), vm );
   boost::program_options::notify( vm );

   std::string projectFolder;
   if( !vm["projectFolder"].empty() )
   {
      projectFolder = vm["projectFolder"].as<std::string>();
   }

   if( vm.count("help") ||
       ( vm["projectFolder"].empty() && vm["clear2DAttributes"].empty() ) ||
       ( vm["projectFolder"].empty() && !vm["clear2DAttributes"].empty() ) ||
       ( !vm["projectFolder"].empty() && !vm["clear2DAttributes"].empty() ) )
   {
#ifdef _WIN32
      projectFolder = guiWindow::getFolder();
#endif
      if( projectFolder.empty() )
      {
         std::cout << desc << std::endl;
         return 0;
      }
   }

   Anonymizer anonymizer;
   if( !projectFolder.empty() )
   {
      anonymizer.setShiftCoordinatesFlag( vm["shiftCoord"].as<bool>() );
      const bool rc = anonymizer.run( projectFolder );
      return rc ? 0 : 1;
   }
#ifndef _WIN32
   else if( !vm["clear2DAttributes"].as<std::string>().empty() )
   {
      anonymizer.removeAttributesFrom2DOutputFile( vm["clear2DAttributes"].as<std::string>() );
      return 0;
   }
#endif
}
