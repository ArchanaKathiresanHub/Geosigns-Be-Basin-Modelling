// Touchstone Include files
//
#define MAX_RUNS 3
#include <string>

#include "MasterTouch.h"
#include "misc.h"
#include "TimeToComplete.h"
#include "BurialHistory.h"
#include "WriteBurial.h"
#include "TouchstoneFiles.h"

#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/PropertyValue.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Snapshot.h"


double MinimumAll (double myValue);

using namespace std;
using namespace fasttouch;
using namespace DataAccess;
using namespace Interface;

#include <cerrno>
#include <cstdlib>
#include <libgen.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

string getPath( ) {

	char buff[1024];

	ssize_t len = readlink("/proc/self/exe",	buff,	sizeof(buff)-1);

	buff[len]= '\0';

	return string(dirname(buff));

}

void executeWrapper( const char * burHistFile, const string & filename, const char * resultFile ) {

	char status[L_tmpnam];
	tmpnam(status);
	mkfifo(status, 0777);

	int pid = fork();

	if (pid == 0) {	

		const char * wrapperName = "touchstoneWrapper.sh";

		errno = 0;
		execl( (getPath() + "/" +  wrapperName).c_str(), wrapperName,
			burHistFile , filename.c_str() , resultFile , status,  
			static_cast<const char *>(0)
			);

		if (errno != 0) {

			cerr << "\n---------------------------------------\n";
			cerr << "  Could not run TouchstoneWrapper '" << getPath() << '/' << wrapperName << "'\n";
			cerr << "  Error code " << errno << ":  " << std::strerror(errno) << '\n'; 
			cerr << "---------------------------------------" << std::endl;

		}
	} 	
	else 	
	{ 
		double fractionCompleted = 0.0; 
		FILE * statusFile = fopen(status,"r");   
		utilities::TimeToComplete timeToComplete( 30, 300, 0.1, 0.1 );
		timeToComplete.start();   

		while (! waitpid(pid, NULL, WNOHANG)) 
		{
			fread(&fractionCompleted,sizeof(fractionCompleted),1,statusFile);
			string reported = timeToComplete.report( MinimumAll(fractionCompleted) );
			if ( !reported.empty()) 
			{
				ReportProgress( reported );
			}     	
		}
		fclose(statusFile);
	}  
}



// initialise statics for user interface
const char* iSd_str             = {"Summary Standard Deviation"};
const char* iMean_str           = {"Summary Mean"};
const char* iSkewness_str       = {"Summary Skewness"};
const char* iKurtosis_str       = {"Summary Kurtosis"};
const char* iMin_str            = {"Summary Minimum"};
const char* iMax_str            = {"Summary Maximum"};
const char* iMode_str           = {"Summary Mode"};
const char* iPercentile_str     = {"Percentile"};
const char* iDistribution_str   = {"Distribution"};
const char* iCore_equiv_str     = {"Porosity Core Equivalent"};
const char* iIntergranular_str  = {"Porosity Intergranular Volume"};
const char* iMacro_str          = {"Porosity Macro"};
const char* iMicro_str          = {"Porosity Micro"};
const char* iAbsolute_str       = {"Permeability Absolute"};
const char* iLog_str            = {"Permeability Log10"};
const char* iCement_Quartz_str  = {"Cement Quartz"};


//const int MasterTouch::numberOfTouchstoneProperties = 6;

// PUBLIC METHODS
//
/// The job of the constructor is to open the ResQ library and initialise
/// the format and category string lists
MasterTouch::MasterTouch( ProjectHandle & projectHandle ) 
   : m_projectHandle( projectHandle )
   , m_percentPercentileMapping()
   , m_categoriesMapping()
   , m_formatsMapping()
   , m_fileList()
   , m_usedSnapshotsIndex()
   , m_usedSnapshotsAge()
{
   // set format mapping
   m_formatsMapping[iSd_str]           = SD;  
   m_formatsMapping[iMean_str]         = MEAN;  
   m_formatsMapping[iSkewness_str]     = SKEWNESS;   
   m_formatsMapping[iKurtosis_str]     = KURTOSIS;    
   m_formatsMapping[iMin_str]          = MIN;          
   m_formatsMapping[iMax_str]          = MAX;          
   m_formatsMapping[iMode_str]         = MODE;         
   m_formatsMapping[iPercentile_str]   = PERCENTILE;  
   m_formatsMapping[iDistribution_str] = DISTRIBUTION;

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
   m_categoriesMapping [iCore_equiv_str]    = 0; //TSLIB_RC_CORE_PORO;
   m_categoriesMapping [iIntergranular_str] = 1; //TSLIB_RC_IGV;
   m_categoriesMapping [iMacro_str]         = 2; //TSLIB_RC_MACRO_PORO;
   m_categoriesMapping [iMicro_str]         = 3; //TSLIB_RC_MICRO_PORO;
   m_categoriesMapping [iAbsolute_str]      = 4; //TSLIB_RC_PERM;
   m_categoriesMapping [iCement_Quartz_str] = 5; //TSLIB_RC_CMT_QRTZ;
   m_categoriesMapping [iLog_str]           = 6; //TSLIB_RC_LOGPERM;
   
   // Used snapshots
	Interface::SnapshotList * MajorSnapshots = m_projectHandle.getSnapshots (Interface::MAJOR);
	Interface::SnapshotList::iterator it;
	
	for (size_t majorSnapshotIndex = 0; majorSnapshotIndex < MajorSnapshots->size(); ++majorSnapshotIndex)
	{
	   if ((*MajorSnapshots)[majorSnapshotIndex]->getUseInResQ())
	   {
	   	m_usedSnapshotsIndex.push_back( majorSnapshotIndex );
	      m_usedSnapshotsAge.push_back( (*MajorSnapshots)[majorSnapshotIndex]->getTime() );
	   }
	}
	
	// Age 0 is always included
	if (m_usedSnapshotsAge.empty() || find( m_usedSnapshotsAge.begin( ), m_usedSnapshotsAge.end( ), 0.0) == m_usedSnapshotsAge.end( )) 
	{
		m_usedSnapshotsIndex.push_back( 0 );
		m_usedSnapshotsAge.push_back( 0.0 );  
	}
		
	delete MajorSnapshots;	
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
   if ( m_fileList.empty() ) 
   { 
      return false;
   }
	
   FileLayerCategoryMapInfoList::const_iterator it;

   for ( it = m_fileList.begin(); it != m_fileList.end(); ++it )
   {
      const string & filename = (it->first);

      // for each TCF file
      LayerCategoryMapInfoList * m_layerList = &(m_fileList[filename]);
      
      LayerCategoryMapInfoList::iterator outIt;
      for( outIt = m_layerList->begin(); outIt != m_layerList->end(); ++outIt )
      {
      	bool calculated = false;
      	for (int runs = 0; runs < MAX_RUNS && !calculated; ++runs) 
      	{
         	
         	// set current layer iterator so can retrieve all output 
         	// combinations during write/save maps
         	CategoryMapInfoList currentOutputs = outIt->second;

         	string progressString = "Starting TCF: ";
         	progressString += filename;
         	progressString += ", Surface: ";
         	progressString += (outIt->first).surface->getName();
         	progressString += ", Formation: ";
         	progressString += (outIt->first).formation->getName();
         	ReportProgress( progressString );

         	// execute TS calculate
         	retrieveGridMaps (currentOutputs);
         	calculated = calculate( filename, (outIt->first).surface, currentOutputs) ;
         	restoreGridMaps (currentOutputs);
         	
         	if (calculated) 
         	{
         		//cout << "MasterTouch::calculate completed successfully " << endl;
         		
         		progressString = "Finished TCF: ";
         		progressString += filename;
         		progressString += ", Surface: ";
         		progressString += (outIt->first).surface->getName();
         		progressString += ", Formation: ";
         		progressString += (outIt->first).formation->getName();

         		ReportProgress( progressString );
				} else 
				{
					cerr << "MasterTouch::calculate is restarted on MPI process " << GetRank( ) << " after " << runs <<" runs"<<endl;
         	}
         }

      }
      // check for early exit from For loop
      if ( outIt != m_layerList->end() ) { break; }
   }        
   
	while (MinimumAll (10) < 10 );
   
   return true;
}  
 
/** Each set of results requested by the user corresponds to a grid map
 *  generated by Cauldron, filled with ResQ calculation results.
 *  Each grid map created must correspond to a Tcf file, a specific layer,
 *  a results category and a result format.
 */
 
bool MasterTouch::addOutputFormat( const string & filename,
                                   const Surface * surface, const Formation * formation, 
                                   const string & category, const string & format,
                                   int percent )
{
   if ( filename.size() < 1 )
   {
       cerr << endl << "Warning, a tcf file has not been chosen" << endl;
       return false;
   }
 
   string propertyValueName;
   propertyValueName += "Resq: ";
   propertyValueName += filename;
   propertyValueName += " ";
   propertyValueName += category;
   propertyValueName += " ";
   propertyValueName += format;

   char perc_str[5];
   sprintf (perc_str, "%d", (int) percent);

   propertyValueName += " ";
   propertyValueName += perc_str;
   
   // create map info
   MapInfo map; 
   map.format  = format;
   map.percent = percent;
     
   // update map.gridMap
   
	for(size_t it = 0; it < m_usedSnapshotsAge.size(); ++it ) 
	{		
	
		const Snapshot * majorSnapshot = m_projectHandle.findSnapshot( m_usedSnapshotsAge[it] );
		      
		if ( !majorSnapshot ) 
		{ 
      cerr << endl << "Could not create PropertyValue: " << propertyValueName  << ", could not find snapshot " << m_usedSnapshotsAge[it] << endl; 
      return false; 
		}
		
		PropertyValue * propertyValue = m_projectHandle.createMapPropertyValue( propertyValueName, majorSnapshot, 0, formation, surface);
		
		if ( !propertyValue )
   	{ 
      cerr << endl << "Could not create PropertyValue named: " << propertyValueName  <<endl; 
      return false; 
   	}
		
		GridMap * gridMap = propertyValue->getGridMap();
		
		map.gridMap.push_back(gridMap);		
   }
   
   LayerInfo layer( surface, formation );

   // add layer info and map info to output list
   m_fileList[filename][layer][category].push_back( map );

   return true;
}

/** This function controls the build of the Cauldron - ResQ functionality. 
 *  Firstly, it creates a burial history for the user-chosen layer through
 *  the Burial History class. THen for each grid point in the layer, it called th
 *  ResQ::calculate function, passing it the burial history data for that particular
 *  coordinate and the Tcf file associated with that layer, to generate all ResQ
 *  results for that particular coordinate. Finally it writes the results to a result
 *  list which is saved to file at a later stage.
 */
bool MasterTouch::calculate( const std::string & filename, const Surface * surface, const CategoryMapInfoList & currentOutputs)
{
   // for each defined node on reservoir surface  
   int firstI = m_projectHandle.getActivityOutputGrid()->firstI();
   int firstJ = m_projectHandle.getActivityOutputGrid()->firstJ();
   int lastI  = m_projectHandle.getActivityOutputGrid()->lastI();
   int lastJ  = m_projectHandle.getActivityOutputGrid()->lastJ();
	
   char burhistFile[L_tmpnam]; 
   tmpnam(burhistFile);

   //Create an write burial history
   {
      BurialHistory burialHistory(surface, m_projectHandle);

      WriteBurial WriteBurial(burhistFile);
      WriteBurial.writeIndexes(firstI, lastI, firstJ, lastJ );
      WriteBurial.writeSnapshotsIndexes(m_usedSnapshotsIndex);
		
		//Write Burial History
		
      for ( int i = firstI; i <= lastI; ++i )
      {
         for( int j = firstJ; j <= lastJ; ++j )
         {
            const std::vector<BurialHistoryTimeStep> & burHistTimesteps 
               = burialHistory.returnAsArray( i, j, true );
            int iD = ( i - firstI )  * ( lastJ - firstJ + 1 ) + j;  
            WriteBurial.writeBurialHistory(burHistTimesteps, iD) ;
         }
      }
   }
   
   char resultFile[L_tmpnam];
   tmpnam(resultFile); 

   //Execute touchstoneWrapper
   try
	{
   	executeWrapper(burhistFile, filename, resultFile);
   }
   catch ( ... )
   {
		cerr << "Message warning: Touchstone wrapper failed for unknown reason " << endl;
      return false;
   }

   //read touchstone categories
   TouchstoneFiles ReadTouchstone(resultFile);
   std::vector<int> vec;
   ReadTouchstone.readOrder(vec);

   m_categoriesMapping[iCore_equiv_str]    = vec[0]; // TSLIB_RC_CORE_PORO;
   m_categoriesMapping[iIntergranular_str] = vec[1]; // TSLIB_RC_IGV;
   m_categoriesMapping[iMacro_str]         = vec[2]; // TSLIB_RC_MACRO_PORO;
   m_categoriesMapping[iMicro_str]         = vec[3]; // TSLIB_RC_MICRO_PORO;
   m_categoriesMapping[iAbsolute_str]      = vec[4]; // TSLIB_RC_PERM;
   m_categoriesMapping[iCement_Quartz_str] = vec[5]; // TSLIB_RC_CMT_QRTZ;
    
   //Read touchstone results
   try 
   {
   	for ( int i = firstI; i <= lastI; ++i )
   	{
      	for( int j = firstJ; j <= lastJ; ++j )
      	{         
         	size_t numTimeSteps = 0;
         	ReadTouchstone.readNumTimeSteps(&numTimeSteps);
         
         	if (numTimeSteps > 0) 
         	{ 
            	for( size_t sn = 0; sn < m_usedSnapshotsIndex.size(); ++sn ) writeResultsToGrids( i, j, currentOutputs, ReadTouchstone, sn);  
         	}
         
      	}
   	} 	
   }	
   catch ( std::exception & e ) 
   {
		cerr << "Message warning: results are not read correctly on MPI process " << GetRank( ) << " : "<< e.what() << endl;
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
void MasterTouch::writeResultsToGrids( int i, int j, const CategoryMapInfoList & currentOutputs, TouchstoneFiles & ReadTouchstone, size_t sn)
{
   const int numberOfTouchstoneProperties = 7;
   const int numberOfStatisticalOutputs   = 29;
   std::vector<double> outputProperties( numberOfTouchstoneProperties * numberOfStatisticalOutputs, 99999.0 );
  
   //Read results
   ReadTouchstone.readArray(outputProperties);

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
         // if resultFormat = {SD = 0, 1, 2, 3, 4, 5, MODE = 6} then resultStat = resultFormat.
         // if resultFormat = { MODE } then resultStat = 7, ... 27 
         // if resultFormat = { DISTRIBUTION } then resultStat = 28;
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



