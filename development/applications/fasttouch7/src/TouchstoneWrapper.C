#include "Touchstone.h"

int main( int argc, char ** argv)
{
   		
   //create touchstone object and load TsLib  
   TouchstoneWrapper ts(argv[1], argv[2], argv[3], argv[4]);
  
   ts.loadTcf( ); 
   
   //calculate and write touchstone results
   ts.calculateWrite( ); 
   
   return 0;
   
}
