#include "anonymizer.h"
#include <iostream>

int main( int argc, char *argv[] )
{
   if( argc == 1 )
   {
      std::cerr << "Invalid usage: provide the folder containing the project file" << std::endl;
      return -1;
   }
   else
   {
      Anonymizer anonymizer;
      const bool rc = anonymizer.run( std::string(argv[1]) );
      return rc ? 0 : -1;
   }
}
