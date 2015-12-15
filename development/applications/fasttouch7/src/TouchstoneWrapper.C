#include "Touchstone.h"
#include <stdlib.h>

int main( int argc, char ** argv)
{ 		
   try
   {
      if ( argc > 5 )
      {
         //create touchstone object and load TsLib  
         TouchstoneWrapper ts( argv[1] 									// burhistFile
                               , argv[2] 								// TCF filename
                               , argv[3] 								// results
                               , argv[4] 								// status
                               , argv[5] 							        // rank
                               , (argc > 6 ? atol( argv[6] ) : 0 )// verbose level
                               );
         //load TCF
         ts.loadTcf(); 
		   
         //calculate and write touchstone results
         ts.calculateWrite();
      }
      else throw TouchstoneWrapper::Exception() << "Wrong parameters number";
   }
   catch ( const TouchstoneWrapper::Exception & e )
   {
      std::cerr << "MeSsAgE error " << e.what() << std::endl;
      return -1;
   }

   return 0;
}
