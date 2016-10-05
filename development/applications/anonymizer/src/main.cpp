#include "anonymizer.h"

int main( int argc, char *argv[] )
{
   Anonymizer anonymizer;
   anonymizer.run( std::string(argv[1]) );
   return 0;
}
