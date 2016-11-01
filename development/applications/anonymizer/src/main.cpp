#include "anonymizer.h"
#include <iostream>

int main( int argc, char *argv[] )
{
   if( argc != 3 )
   {
      std::cerr << "Invalid usage:" << std::endl;
      std::cerr << " -projectFolder <folder>" << std::endl;
#ifndef _WIN32
      std::cerr << " -clear2DAttributes <HDF file>" << std::endl;
#endif
      return -1;
   }
   else
   {
      Anonymizer anonymizer;
      const std::string mode(argv[1]);
      if( mode.compare(0,mode.size(),"-projectFolder") == 0 )
      {
         const bool rc = anonymizer.run( std::string(argv[2]) );
         return rc ? 0 : -1;
      }
#ifndef _WIN32
      else if( mode.compare(0,mode.size(),"-clear2DAttributes") == 0 )
      {
         anonymizer.removeAttributesFrom2DOutputFile( std::string(argv[2]) );
         return 0;
      }
#endif
   }
}
