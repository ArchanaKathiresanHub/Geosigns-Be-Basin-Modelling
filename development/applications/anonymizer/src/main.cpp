#include "anonymizer.h"
#include <iostream>
#include "boost/program_options.hpp"

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

   if( vm.count("help") ||
       ( vm["projectFolder"].empty() && vm["clear2DAttributes"].empty() ) ||
       ( vm["projectFolder"].empty() && !vm["clear2DAttributes"].empty() ) ||
       ( !vm["projectFolder"].empty() && !vm["clear2DAttributes"].empty() ) )
   {
      std::cout << desc << std::endl;
      return 0;
   }

   Anonymizer anonymizer;
   const std::string projFolder( vm["projectFolder"].as<std::string>() );
   if( !projFolder.empty() )
   {
      anonymizer.setShiftCoordinatesFlag( vm["shiftCoord"].as<bool>() );
      const bool rc = anonymizer.run( projFolder );
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
