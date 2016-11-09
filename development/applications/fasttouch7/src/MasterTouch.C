// Touchstone Include files
//
#include <string>
#include <limits.h>
#include <fcntl.h>

#include "MasterTouch.h"
#include "misc.h"
#include "TimeToComplete.h"
#include "TouchstoneFiles.h"

#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/PropertyValue.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Snapshot.h"
#include "Interface/RunParameters.h"

#include "Path.h"

double MinimumAll (double myValue);

using namespace std;
using namespace fasttouch;
using namespace DataAccess;
using namespace Interface;

#include <cerrno>
#include <cstdlib>
#include <libgen.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/stat.h>

static const char * tempBurial  = "/tmp/BurialhistXXXXXX";
static const char * tempResults = "/tmp/ResultsXXXXXX";
static const char * tempStatus  = "/tmp/StatusXXXXXX";

bool MasterTouch::checkZombie( pid_t pid )
{
#ifdef _WIN32
   return false;
#else
   char pbuf[32];
      
   snprintf( pbuf, sizeof( pbuf ), "/proc/%d/stat", (int)pid );
   FILE* fpstat = fopen( pbuf, "r" );
	
   // if "/proc/%d/stat" is not found we can not check the status of the zombie  	
   if ( !fpstat ) return false;
	
   int  rpid = 0;
   char rcmd[32];
   char rstatc = 0;
	
   fscanf( fpstat, "%d %30s %c", &rpid, rcmd, &rstatc ); 
   fclose(fpstat);
   return rstatc == 'Z' ? true : false;
#endif
}

bool MasterTouch::executeWrapper( const char * burHistFile, const string & filename, const char * resultFile ) {

   char status[PATH_MAX];
   strcpy( status, tempStatus );
   mkstemp( status );
   mkfifo( status, 0777 );

   ostringstream rank;
   rank << GetRank();

   int pid = fork();
   int childStatus = 0;

   if (pid == 0)
   {
      const char * wrapperName = "touchstoneWrapper";

      errno = 0;
      ibs::Path pathToWrapper = ibs::Path::applicationFullPath();
      pathToWrapper << wrapperName;

      // create a temporary file in the current directory to store the standard output (e.g. the warning messages from Matlab)
      string wrapperOut("./wrapperStandardOutput_");
      wrapperOut += rank.str( );
      //read and write permissions for the user, read permission for the group  
      mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
      // copy standard output (1) to file
      int fd=open(wrapperOut.c_str(),O_WRONLY| O_CREAT| O_TRUNC, mode);
      dup2(fd,1);
      close(fd);

      if ( pathToWrapper.exists() )
      {
         execl( pathToWrapper.path().c_str(), wrapperName,
            burHistFile, filename.c_str() , resultFile , status, (rank.str()).c_str(),
            static_cast<const char *>(0) );

         if (errno != 0)
         {
            std::ostringstream oss;
            oss << "error: Could not run TouchstoneWrapper '" << pathToWrapper.path()
               << "  Error code " << errno << ":  " << std::strerror(errno);
            message( oss.str());
         }
      }
      else
      {
         message( (std::string( "error: could not find TouchstoneWrapper at: ") + pathToWrapper.path()).c_str() );
      }
      exit( 0 );
   }

   double fractionCompleted = -10.0;
   double temFraction = -10.0;
   double minAllFractions  = 0.01;
   double currentMinimum = 0.0;
   bool childStarted = false;
   int byteRead;
   int childstate;
   utilities::TimeToComplete timeToComplete( 5, 30, minAllFractions, 0.01 );
   int fd = open( status, O_RDONLY| O_NONBLOCK );
   timespec sleepTime;
   timespec remainingTime;
   
   sleepTime.tv_sec =  0;
   sleepTime.tv_nsec = 5000000L;

   ReportProgress( "Loading touchstone and creating the realizations, please wait ..." ); 
      
   // here we check several behaviours that can happen in the loading phase. 
   // If one child is bad we re-start all childrens. 
   do
   {
      // read the fractionCompleted first
      byteRead = read( fd, &temFraction, sizeof( temFraction ) );
      if ( byteRead == sizeof( temFraction ) ) fractionCompleted = temFraction;
      
      // get the waitpidReturnValue and the childstate
      bool waitpidReturnValue = waitpid( pid, &childstate, WNOHANG );

      // the message string stream
      ostringstream oss;

      // if the child did not exit and computation started it is a good one
	  // in case of no active elements in the domain, no computations are performed and fractionCompleted == 1
      if ( (!waitpidReturnValue && fractionCompleted > 0.0) || (waitpidReturnValue && fractionCompleted == 1.0)  )
      {
         childStarted = true;
      } 
      
      // if the child did not exit and is zombie it is a bad child
      else if ( !waitpidReturnValue && fractionCompleted < 0.0 && checkZombie( pid ) )
      {
         oss << "warning: touchstoneWrapper is zombie " << std::strerror( errno );
         message( oss.str( ) );
         childStatus = -1;
		   childStarted = true;
      } 

      // if the child exited badly before starting the computations it is a bad child
      else if ( waitpidReturnValue && fractionCompleted < 0.0 && !WIFEXITED( childstate ))
      {
         oss << "warning: touchstoneWrapper terminated prematurely "<< std::strerror( errno );
         message( oss.str( ) );
         childStatus = -1;
		   childStarted = true;
      } 
      
      // if the child exited ok before starting the computations it is a bad child (an exception was thrown and caught in Touchstone.C)
      else if ( waitpidReturnValue && fractionCompleted < 0.0 && WIFEXITED( childstate ))
      {
         oss << "warning: touchstoneWrapper thrown an exeception "<< std::strerror( errno );
         message( oss.str( ) );
         childStatus = -1;
		   childStarted = true;
      }
	  
      // sleep only after the checks ( waitpidReturnValue and childstate are up to date in the checks above). 
      nanosleep( &sleepTime, &remainingTime  );	  
   } while ( !childStarted );
   
   currentMinimum = MinimumAll( childStatus );
   
   // return false if currentMinimum is < 0.0 
   if (currentMinimum < 0.0 ) return false; 
         
   timeToComplete.start( );      
   
   // Progress is reported only by Rank 0, we need to keep Rank 0 in the loop even if its child exited. 
   // One way to do this is to exit the while loop only if the child did not exit AND we are done in all ranks (currentMinimum < 1.0)
   while ( !waitpid( pid, &childstate, WNOHANG ) || currentMinimum < 1.0 )
   {
      // read the fractionCompleted first. Calling MinimumAll requires all MPI processes to sync all times. This is expensive so we call it every 0.005 sec
      byteRead = read( fd, &temFraction, sizeof( temFraction ) );
      if ( byteRead == sizeof( temFraction ) ) fractionCompleted = temFraction;

      currentMinimum = MinimumAll( fractionCompleted );
      
      if ( currentMinimum > minAllFractions )
      {
         minAllFractions = currentMinimum;
         string reported = timeToComplete.report( minAllFractions );
         if (!reported.empty()) ReportProgress( reported );
      }

	   // sleep only after the check
      nanosleep( &sleepTime, &remainingTime  );		
   }       
   if ( close( fd ) < 0 )
   {
      ostringstream oss;
      oss << "error: Could not close status file, error code " << std::strerror(errno);
      message( oss.str() );       
      childStatus = -1;
   }   
   if ( unlink( status ) < 0 )
   {
      ostringstream oss;
      oss << "error: Could not unlink status file, error code " << std::strerror(errno);
      message( oss.str() );
      childStatus = -1;
   }

   // if one child cannot close or unlink the status file, re-start all childrens.
   currentMinimum = MinimumAll( childStatus );

   return currentMinimum < 0.0 ? false : true; 

}
// PUBLIC METHODS
//
/// The job of the constructor is to open the ResQ library and initialise
/// the format and category string lists
MasterTouch::MasterTouch( ProjectHandle & projectHandle ) 
   : m_projectHandle( projectHandle )
   , m_percentPercentileMapping()
   , m_categoriesMapping()
   , m_categories()
   , m_formatsMapping()
   , m_layerList()
   , m_usedSnapshotsIndex()
   , m_usedSnapshotsAge()
   , m_verboseLevel(0)
   , m_layerCategoryResultCounter()
   , m_fileLayerFaciesGridMap()
{
   // set format mapping
   m_formatsMapping["Summary Standard Deviation"]  = SD;  
   m_formatsMapping["Summary Mean"]         	      = MEAN;  
   m_formatsMapping["Geometric Mean"]      	      = GEOMEAN; 
   m_formatsMapping["Summary Skewness"]     	      = SKEWNESS;   
   m_formatsMapping["Summary Kurtosis"]     	      = KURTOSIS;    
   m_formatsMapping["Summary Minimum"]             = MIN;          
   m_formatsMapping["Summary Maximum"]             = MAX;          
   m_formatsMapping["Summary Mode"]         	      = MODE;         
   m_formatsMapping["Percentile"]   		         = PERCENTILE;  
   m_formatsMapping["Distribution"]                = DISTRIBUTION;


   // To default the percent for Percentile to 60 %.
   for ( int i = 0; i < 100; ++i )
   {
      m_percentPercentileMapping [i] = 12;
   }
   m_percentPercentileMapping[ 1 ] = 0;

   // Set now the available percents for Percentile.
   for ( int i = 5; i < 99; i = i + 5 )
   {
      m_percentPercentileMapping [i] = int (i/5);
   }
   m_percentPercentileMapping[ 99 ] = 20; 
   
   // set categories mapping 
   m_categories.push_back("Porosity Macro");
   m_categories.push_back("Porosity Intergranular Volume");
   m_categories.push_back("Cement Quartz");
   m_categories.push_back("Porosity Core Equivalent");
   m_categories.push_back("Porosity Micro");
   m_categories.push_back("Permeability Absolute");
   m_categories.push_back("Permeability Log10");
   
   //default indexing
   m_categoriesMapping[m_categories[MACRO_PORO]]		= MACRO_PORO;	
   m_categoriesMapping[m_categories[IGV]]		        = IGV;	
   m_categoriesMapping[m_categories[CMT_QRTZ]]  		= CMT_QRTZ;	
   m_categoriesMapping[m_categories[CORE_PORO]]   		= CORE_PORO;	
   m_categoriesMapping[m_categories[MICRO_PORO]]   	    = MICRO_PORO;
   m_categoriesMapping[m_categories[PERM]]   			= PERM;		
   m_categoriesMapping[m_categories[LOGPERM]]   		= LOGPERM;
     
   // Used snapshots
   Interface::SnapshotList * majorSnapshots = m_projectHandle.getSnapshots (Interface::MAJOR);
   Interface::SnapshotList::iterator it;
	
   for (size_t majorSnapshotIndex = 0; majorSnapshotIndex < majorSnapshots->size(); ++majorSnapshotIndex)
   {
      if ((*majorSnapshots)[majorSnapshotIndex]->getUseInResQ())
      {
         m_usedSnapshotsIndex.push_back( majorSnapshotIndex );
         m_usedSnapshotsAge.push_back( (*majorSnapshots)[majorSnapshotIndex]->getTime() );
      }
   }
	
   // Age 0 is always included
   if (m_usedSnapshotsAge.empty() || find( m_usedSnapshotsAge.begin( ), m_usedSnapshotsAge.end( ), 0.0) == m_usedSnapshotsAge.end( )) 
   {
      m_usedSnapshotsIndex.push_back( 0 );
      m_usedSnapshotsAge.push_back( 0.0 );  
   }
   
   delete majorSnapshots;	
   
   //verbosity level
   PetscBool inputVerbose;
   PetscInt verboseLevel;
	
   PetscOptionsGetInt (PETSC_NULL, "-verboselevel", &verboseLevel, &inputVerbose);
   if (inputVerbose)
   {
      m_verboseLevel = verboseLevel;
   }
}

/** The writeBurialHistoryToFile is responsable for writing the burial histories to file
*/
void MasterTouch::writeBurialHistoryToFile(const string & filename, const char * burhistFile, const int numActive)
{
   
   LayerFaciesGridMap layerFaciesGridMap = m_fileLayerFaciesGridMap[filename]; 
   WriteBurial writeBurial(burhistFile);
   
   // for each defined node on reservoir surface  
   int firstI = m_projectHandle.getActivityOutputGrid()->firstI();
   int firstJ = m_projectHandle.getActivityOutputGrid()->firstJ();
   int lastI  = m_projectHandle.getActivityOutputGrid()->lastI();
   int lastJ  = m_projectHandle.getActivityOutputGrid()->lastJ();
	   	
   writeBurial.writeIndexes(firstI, lastI, firstJ, lastJ, layerFaciesGridMap.size( ),numActive);
   writeBurial.writeSnapshotsIndexes(m_usedSnapshotsIndex);
			
   //Second reading of the burial history			
   LayerFaciesGridMap::iterator outIt;
   for( outIt = layerFaciesGridMap.begin(); outIt != layerFaciesGridMap.end(); ++outIt )
   {			
      writeBurialHistory( (outIt->first).surface, writeBurial, &m_fileLayerFaciesGridMap[filename][outIt->first]);
   }
}

/** The run function is responsible for carrying out the functional
 *  steps involved in using the ResQ library.
 *  So firstly, a request is sent to the ResQ library to open all the user-selected
 *  Tcf files. Then for each Tcf file, all calculations and saving of results
 *  are executed through the functions: calculate and saveGrids. Lastly, the 
 *  Tcf files are closed and dynamic memory is cleaned up.
 */
bool MasterTouch::run()
{   
   // make sure data has been selected 
   if ( m_fileLayerFaciesGridMap.empty() ) 
   { 
      return false;
   }
	
   // for each TCF file
   bool failure = false;
   FileLayerFaciesGridMap::iterator it;
   for ( it = m_fileLayerFaciesGridMap.begin(); it != m_fileLayerFaciesGridMap.end(); ++it )
   {		
      const string & filename = (it->first);   
      LayerFaciesGridMap  layerFaciesGridMap = m_fileLayerFaciesGridMap[filename];  
       
      // First reading of the burial history to count the total number of active timesteps in each layer. This is relativly fast.
      int numActive = 0;
      LayerFaciesGridMap::iterator outIt;
      for( outIt = layerFaciesGridMap.begin(); outIt != layerFaciesGridMap.end(); ++outIt )
      {			
	      numActive += countActive( (outIt->first).surface, &m_fileLayerFaciesGridMap[filename][outIt->first]);
      } 	
	  
      string progressString = "Starting TCF: ";
      progressString += filename;
      ReportProgress( progressString );

	  //write burial histories for all layers and facies that uses that TCF file
	  char burhistFile[PATH_MAX];
	  strcpy(burhistFile, tempBurial);
	  mkstemp(burhistFile);
	  
	  //write the burial history
	  writeBurialHistoryToFile(filename, burhistFile, numActive);
	  
      // run touchstone wrapper      
      // check if failure needs to be simulated
      char * touchstoneWrapperFailure = getenv ( "touchstoneWrapperFailure" );      
     
      bool calculated = false;   			
      for (int runs = 1; runs <= MAX_RUNS && !calculated; ++runs) 
      {
         
         if (touchstoneWrapperFailure && GetRank() == atol(touchstoneWrapperFailure)) 
         {
         calculated = calculate("WrongTCF", burhistFile);
         }
         else
         {
         calculated =  calculate(filename, burhistFile);
         }
  
         if (!calculated) 
         {
            std::ostringstream oss;
            oss << "warning: MasterTouch::calculate is restarted on MPI process " << GetRank( ) << " after " << runs <<" runs";
            message( oss.str() );
         }		
      }

      while(MinimumAll(10)<10);
                    
      if (!calculated) 
      {
         failure = true;
         break;
      }
      
      progressString = "Finished TCF: ";
      progressString += filename;
      ReportProgress( progressString ); 
   }        
  
   return (!failure);
   
}  
 
/** Each set of results requested by the user corresponds to a grid map
 *  generated by Cauldron, filled with ResQ calculation results.
 *  Each grid map created must correspond to a specific layer and results category + format
 */
 
bool MasterTouch::addOutputFormat( const string & filename,
                                   const Surface * surface, const Formation * formation, 
                                   const string & category, const string & format,
                                   int percent, const GridMap * faciesGrid, int index )
{
   if ( filename.size() < 1 )
   {
      ostringstream oss;
      oss << "warning: MasterTouch::addOutputFormat, a tcf file has not been chosen ";   	
      message( oss.str() ); 
      return false;
   }
 
   string propertyValueName;
   propertyValueName += "Resq: ";  
   const bool isLegacy = m_projectHandle.getRunParameters()->getLegacy();
   if (isLegacy)
   {
   propertyValueName += filename;
   propertyValueName += " ";
   }
   propertyValueName += category;
   propertyValueName += " ";
   propertyValueName += format;

   propertyValueName += " ";
   propertyValueName += to_string(percent);
   
   LayerInfo layer( surface, formation );
   
   //check if a map for the layer already exists for propertyValueName
   if (m_layerCategoryResultCounter[layer].count(propertyValueName) == 0)    
   {
   
      //Create map info
      MapInfo map; 
      map.format  = format;
      map.percent = percent;
   
      for(size_t it = 0; it < m_usedSnapshotsAge.size(); ++it ) 
      {		
	
         const Snapshot * majorSnapshot = m_projectHandle.findSnapshot( m_usedSnapshotsAge[it] );
		      
         if ( !majorSnapshot ) 
         { 
            ostringstream oss;
            oss << "warning: MasterTouch::addOutputFormat, Could not create PropertyValue: " << propertyValueName  << 
               ", could not find snapshot " << m_usedSnapshotsAge[it]; 	
            message( oss.str() );          
            return false;  
         }
		
         PropertyValue * propertyValue = m_projectHandle.createMapPropertyValue( propertyValueName, majorSnapshot, 0, formation, surface);
		
         if ( !propertyValue )
         { 
            ostringstream oss;
            oss << "warning: MasterTouch::addOutputFormat, Could not create PropertyValue named: " << propertyValueName;
            message( oss.str() ); 
            return false; 
         }
		
         GridMap * newGridMap = propertyValue->getGridMap();		
         map.gridMap.push_back(newGridMap);		
      }
   
      // add layer info and map info to output list
      m_layerList[layer][category].push_back( map );
      m_layerCategoryResultCounter[layer][propertyValueName] = 1;
   
   }
 
   //check if a FaciesGridMap for the TCF already exists for the layer
   if (m_fileLayerFaciesGridMap[filename].count(layer) == 0)
   {    
      faciesGridMap faciesGridMap; 
      faciesGridMap.GridMap = faciesGrid;
      faciesGridMap.faciesNumber = index;
      m_fileLayerFaciesGridMap[filename][layer] = faciesGridMap ;
   }
   
   return true;
}
/** This function counts all active timesteps in the active region **/
int MasterTouch::countActive( const Surface * surface, const faciesGridMap * faciesGridMap)
{

   // for each defined node on reservoir surface  
   int firstI = m_projectHandle.getActivityOutputGrid()->firstI();
   int firstJ = m_projectHandle.getActivityOutputGrid()->firstJ();
   int lastI  = m_projectHandle.getActivityOutputGrid()->lastI();
   int lastJ  = m_projectHandle.getActivityOutputGrid()->lastJ();
      
   bool facieGridMapisDefined = false;
   bool writeFlag = true;
   double gridMapValue = -1.0;
   int numActive = 0;
	
   // if a facies is not defined, the entire surface belongs to the TCF   
   if (faciesGridMap->GridMap) 
   {
      facieGridMapisDefined = true;
      faciesGridMap->GridMap->retrieveData(false);
   }
  
   // retrive burial history 	  
   BurialHistory burialHistory(surface, m_projectHandle);
       
   // write Burial History	
   for ( int i = firstI; i <= lastI; ++i )
   {
      for( int j = firstJ; j <= lastJ; ++j )
      {
         // write burial histories only for selected areas, for areas with gridMapValue == 0 do not write burial histories. Perform this check only if facieGridMapisDefined    
         if (facieGridMapisDefined) 
         {
            gridMapValue = faciesGridMap->GridMap->getValue((unsigned int) i, (unsigned int)j);
            if (gridMapValue == 0.0) 
            {
               writeFlag = false ;
            } 
            else
            {
               writeFlag = (gridMapValue == faciesGridMap->faciesNumber);
            }
         }
		 				 				   
        if (!writeFlag) continue;
   
        const std::vector<BurialHistoryTimeStep> & burHistTimesteps = burialHistory.returnAsArray( i, j, true );
      
        for ( size_t bt = 0; bt < burHistTimesteps.size(); ++bt )
        {
          if (burHistTimesteps[bt].temperature == Interface::DefaultUndefinedMapValue ) continue;
          numActive += 1;
        }
      }
   }

   if (faciesGridMap->GridMap) faciesGridMap->GridMap->restoreData(false,false);
   
   return numActive; 
}

/** This function writes burial histories to file for a formation **/

void MasterTouch::writeBurialHistory( const Surface * surface, WriteBurial & writeBurial, const faciesGridMap * faciesGridMap)
{

   // for each defined node on reservoir surface  
   int firstI = m_projectHandle.getActivityOutputGrid()->firstI();
   int firstJ = m_projectHandle.getActivityOutputGrid()->firstJ();
   int lastI  = m_projectHandle.getActivityOutputGrid()->lastI();
   int lastJ  = m_projectHandle.getActivityOutputGrid()->lastJ();
      
   bool facieGridMapisDefined = false;
   bool writeFlag = true;
   double gridMapValue = -1.0;
   
   // if a facies is not defined, all surface belongs to the TCF   
   if (faciesGridMap->GridMap) 
   {
      facieGridMapisDefined = true;
      faciesGridMap->GridMap->retrieveData(false);
   }
  
   // retrive burial history 	  
   BurialHistory burialHistory(surface, m_projectHandle);
       
   // write Burial History	
   for ( int i = firstI; i <= lastI; ++i )
   {
      for( int j = firstJ; j <= lastJ; ++j )
      {
         // write burial histories only for selected areas, for areas with gridMapValue == 0 do not write burial histories. Perform this check only if facieGridMapisDefined    
         if (facieGridMapisDefined) 
         {
            gridMapValue = faciesGridMap->GridMap->getValue((unsigned int) i, (unsigned int)j);
            if (gridMapValue == 0) 
            {
               writeFlag = false ;
            } 
            else
            {
               writeFlag = (gridMapValue == faciesGridMap->faciesNumber);
            }
         }
		 				 				
         const std::vector<BurialHistoryTimeStep> & burHistTimesteps = burialHistory.returnAsArray( i, j, true );
		 	
         int iD = ( i - firstI )  * ( lastJ - firstJ + 1 ) + j;  
                  
         writeBurial.writeBurialHistory(burHistTimesteps, iD, writeFlag) ;
      }
   }
	
   if (faciesGridMap->GridMap) faciesGridMap->GridMap->restoreData(false,false); 
}

/** This function controls the build of the Cauldron - ResQ functionality. 
 *  Firstly, it creates a burial history for the user-chosen layer through
 *  the Burial History class. THen for each grid point in the layer, it called th
 *  ResQ::calculate function, passing it the burial history data for that particular
 *  coordinate and the Tcf file associated with that layer, to generate all ResQ
 *  results for that particular coordinate. Finally it writes the results to a result
 *  list which is saved to file at a later stage.
 */
bool MasterTouch::calculate( const std::string & filename, const char * burhistFile )
{
   try 
   {
      // for each defined node on reservoir surface  
      int firstI = m_projectHandle.getActivityOutputGrid()->firstI();
      int firstJ = m_projectHandle.getActivityOutputGrid()->firstJ();
      int lastI  = m_projectHandle.getActivityOutputGrid()->lastI();
      int lastJ  = m_projectHandle.getActivityOutputGrid()->lastJ();
   
      char resultFile[PATH_MAX];
      strcpy(resultFile,tempResults);
      mkstemp(resultFile);

      //Execute touchstoneWrapper
      if (!executeWrapper( burhistFile, filename, resultFile))
      {
         throw std::runtime_error( "Error in executeWrapper" );
      }
   
      //read touchstone categories
      TouchstoneFiles readTouchstone(resultFile);
      std::vector<int> vec;
      readTouchstone.readOrder(vec);
		
      //as saved by the library
      for ( size_t ii = 0; ii < vec.size() - 1; ++ii ) m_categoriesMapping[m_categories[ii]] = vec[ii];
        
      //Read touchstone results for all included layers	
      LayerCategoryMapInfoList::iterator outIt;
      
      for( outIt = m_layerList.begin( ); outIt != m_layerList.end(); ++outIt )
      {	
         //the considered TCF contains the layers in the lists
         if (m_fileLayerFaciesGridMap[filename].count(outIt->first) > 0) 
         {
         
            CategoryMapInfoList currentOutputs = outIt->second;
            retrieveGridMaps (currentOutputs);
	
            for ( int i = firstI; i <= lastI; ++i )
            {
               for( int j = firstJ; j <= lastJ; ++j )
               {         
                  size_t numTimeSteps = 0;
                  readTouchstone.readNumTimeSteps(&numTimeSteps);
               
                  if (numTimeSteps > 0) 
                  { 
                     for( size_t sn = 0; sn < m_usedSnapshotsIndex.size(); ++sn ) writeResultsToGrids( i, j, currentOutputs, readTouchstone, sn);  
                  }
               }
            }
         
            restoreGridMaps (currentOutputs); 	
         
         }         
      }
      //catch exceptions
   } 
   catch ( const std::runtime_error & e )
   { 
      ostringstream oss;
      oss << "error: MasterTouch::calculate() error on MPI process " << GetRank() << " : "<< e.what();   	
      message( oss.str() ); 
      return false;
   }
   catch ( std::exception & e ) 
   {
      ostringstream oss;
      oss << "error: MasterTouch::calculate() error on MPI process " << GetRank() << " : "<< e.what();   	
      message( oss.str()); 
      return false;
   }
   catch( ... )
   {
      ostringstream oss;
      oss << "error: MasterTouch::calculate() unknown  error on MPI process " << GetRank();
      message( oss.str()); 
      return false;
   } 	
   
   return true;
}

/** For each layer and associated tcf file, there may be more than one result
 *  category and format selected. This means that after the ResQ results are calculated
 *  for a grid point on a certain layer, all the categories and formats associated 
 *  with that particular layer should be extracted before the next grid point's 
 *  results are processed
 */
void MasterTouch::writeResultsToGrids( int i, int j, const CategoryMapInfoList & currentOutputs, TouchstoneFiles & readTouchstone, size_t sn)
{
   const int numberOfTouchstoneProperties = 7;
   const int numberOfStatisticalOutputs   = 30;
   std::vector<double> outputProperties( numberOfTouchstoneProperties * numberOfStatisticalOutputs, 99999.0 );
  
   //Read results
   readTouchstone.readArray(outputProperties);

   // loop through each combination of category and format choices
   // for the current layer    
   // for each category selected
   for ( CategoryMapInfoList::const_iterator it = currentOutputs.begin(); it != currentOutputs.end (); ++it )
   {
      int resultCat = m_categoriesMapping[(*it).first]; 
      // for each format selected
      for ( MapInfoList::const_iterator mIt = ((*it).second).begin(); mIt != ((*it).second).end(); ++mIt )
      {
         // write appropriate results to grid
         int resultFormat = m_formatsMapping[(*mIt).format];
         // Explanation to formula below:
         // if resultFormat = {SD = 0, 1, 2, 3, 4, 5, MODE = 7} then resultStat = resultFormat.
         // if resultFormat = { MODE } then resultStat = 8, ... 29 
         // if resultFormat = { DISTRIBUTION } then resultStat = 30;
         int resultStat = resultFormat 
	    + int (resultFormat > MODE) * ( m_percentPercentileMapping[ (*mIt).percent ] ) 
	    + int (resultFormat > PERCENTILE ) * ( 20 - m_percentPercentileMapping [ (*mIt).percent ] );
         (*mIt).gridMap[sn]->setValue( i, j,  outputProperties[ resultCat *  numberOfStatisticalOutputs + resultStat ] );
      }
   }
}

bool MasterTouch::retrieveGridMaps(const CategoryMapInfoList & currentOutputs)
{
   for ( CategoryMapInfoList::const_iterator cmilIterator = currentOutputs.begin(); cmilIterator != currentOutputs.end(); ++cmilIterator )
   {
      const MapInfoList & mapInfoList = (*cmilIterator).second;
        
      for ( MapInfoList::const_iterator milIterator = mapInfoList.begin(); milIterator != mapInfoList.end(); ++milIterator )
      {
         const MapInfo & mapInfo = *milIterator;
         for(size_t it = 0; it < m_usedSnapshotsIndex.size(); ++it ) 
	    mapInfo.gridMap[it]->retrieveData ();
         
      }
   }
   return true;
}

bool MasterTouch::restoreGridMaps(const CategoryMapInfoList & currentOutputs)
{
   for (CategoryMapInfoList::const_iterator cmilIterator = currentOutputs.begin (); cmilIterator != currentOutputs.end (); ++cmilIterator)
   {
      const MapInfoList & mapInfoList = (*cmilIterator).second;

      for (MapInfoList::const_iterator milIterator = mapInfoList.begin (); milIterator != mapInfoList.end (); ++milIterator)
      {
         const MapInfo & mapInfo = *milIterator;
         for(size_t it = 0; it < m_usedSnapshotsIndex.size(); ++it ) 
	    mapInfo.gridMap[it]->restoreData ();
      }
   }
   return true;
}



