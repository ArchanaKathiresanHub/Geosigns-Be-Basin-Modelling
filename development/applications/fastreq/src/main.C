#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include <iostream>
      using namespace std;
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   using namespace std;
#endif // sgi


#include "FastReqProjectHandle.h"
#include "FastReqObjectFactory.h"
using FastReq::FastReqProjectHandle;
using FastReq::FastReqObjectFactory;

#include <string>
using namespace std;

#include "math.h"

//test example
//fastreq -project ProjectFullPathName OPTIONAL :-fastcauldron|-fastgenex|-fastmig| 
//Example 1: fastreq -project ProjectFullPathName --> All the simulators will be taken into account
//Example 2: fastreq -project ProjectFullPathName -fastcauldron --> Only fastcauldron will be taken into account
//Example 3: fastreq -project ProjectFullPathName -fastgenex    --> Only fastgenex will be taken into account
//Example 4: fastreq -project ProjectFullPathName -fastmig      --> Only fastmig will be taken into account
//Example 5 :fastreq -project ProjectFullPathName -fastcauldron -fastgenex      --> Only fastcauldron AND fastgenex will be taken into account


int main (int argc, char ** argv)
{
   string inputFileName;
   
   //Type of simulator
   enum Simulator{ PressureTemp = 0, Genex = 1, Migration = 2, HighResDecompaction = 3, numberOfSimulators};
   
   double MegaBytesPerNodePerSimulator[numberOfSimulators] = { 0.0026, 0.000484, 0.001, 0.002 };
   //If simulator is included in the commandline option the corresponding entry in SimulatorActive is set to true

   bool SimulatorActive[numberOfSimulators] = {false, false, false, false};
   const std::string ARGUMENTS[numberOfSimulators - 1] = {"-fastcauldron", "-fastgenex", "-fastmig"};
   const double SourceRockNodeOuputBytesPerSnapshot =  0.00108;
   unsigned int NodesPerSimulator[numberOfSimulators] = {0};

   int i;
   
   //process the command line arguments
   if (argc < 3)
   {
      cout<<"No Project Defined...Aborting"<<endl;
      return -1;
   }
   else if(argc >= 3)
   {
      inputFileName = argv[2];
      if(argc == 3)
      {
         //if no arguments for simulator consider all simulators
	      for(i = PressureTemp; i < numberOfSimulators; i++)
	      {
                 SimulatorActive[ i ] = true;
	      }
      }
      else
      {
         for(i = 3; i < argc; i++)
         {
            if(argv[ i ] == ARGUMENTS[PressureTemp] )
            {
               SimulatorActive[ PressureTemp ] = true;
            }
            else if(argv[ i ] == ARGUMENTS[Genex] )
            {
               SimulatorActive[ Genex ] = true;
            }
            else if(argv[ i ] == ARGUMENTS[Migration] )
            {
               SimulatorActive[ Migration ] = true;
               SimulatorActive[ HighResDecompaction ] = true;
            }
         }
      }
   }

   //Create the objectFactory
   FastReqObjectFactory *theFactory = 0;
   theFactory = new FastReqObjectFactory;
   assert(theFactory);
   //Create the projectHandle
   DataAccess::Interface::ProjectHandle::UseFactory (theFactory);
   FastReqProjectHandle *theProjectHandle = FastReqProjectHandle::CreateFrom(inputFileName);

   //Compute nodes per simulator
   if (SimulatorActive[ PressureTemp ] )
   {
      //cerr<<"Pressure active "<<endl;
      NodesPerSimulator[PressureTemp]        = theProjectHandle->getNumberOfPressureTemperatureNodes();
      //cerr<<"Nodes for Pressure "<<NodesPerSimulator[PressureTemp]<<endl;
   }
   if (SimulatorActive[ Genex ] )
   {
       //484 + 1080*Snapshots
       //2do: need to take into account ONLY the switched on genex properties not all of them
       //484 + (108 + Number Of Optional Genex Properties) * 8 * NumberOfSnapshots, now NumberOfOptionalProperties=27
       //cerr<<"Genex active "<<endl;
       NodesPerSimulator[Genex]               = theProjectHandle->getNumberOfGenexNodes();
       //cerr<<"Nodes for Genex "<<NodesPerSimulator[Genex]<<endl;
       MegaBytesPerNodePerSimulator[Genex] += SourceRockNodeOuputBytesPerSnapshot * theProjectHandle->getNumberOfSnapShots();
       //cerr<<"Snapshots for Genex "<<theProjectHandle->getNumberOfSnapShots()<<endl;
   }
   if (SimulatorActive[ Migration ] )
   {
       NodesPerSimulator[Migration]           = theProjectHandle->getNumberOfMigrationNodes();
       NodesPerSimulator[HighResDecompaction] = theProjectHandle->getNumberOfHighResDecompactionNodes();
   }

   //find the maximum
   double maximumMemory = 0.0;
  
   for( i = PressureTemp; i < numberOfSimulators; i++)
   { 
      if (SimulatorActive[ i ] )
      {
         double currentMemory = MegaBytesPerNodePerSimulator[ i ] * static_cast<double>( NodesPerSimulator [ i ] );
         if ( currentMemory > maximumMemory )
         {
               maximumMemory = currentMemory;
         }
	
      }
   }
   unsigned int output = static_cast<unsigned int>( ceil(maximumMemory) ) ;
   
   if(output < 100)
   {
      output = 100;
   }
   
   cout<<"MeSsAgE DATA MEMORY "<<output<<endl;

   delete theProjectHandle;
   delete theFactory;
 
   return 0;
}
