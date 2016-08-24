#include "Touchstone.h"
#include "ReadBurial.h"
#include "TouchstoneFiles.h"

#include <iostream>
#include <cmath>
#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

using namespace Geocosm;

/** Mapping categories. */
int TouchstoneWrapper::SaveResultHeader( TcfSchema::DetailHeadersType::modalHeaders_iterator& itor )
{
   for ( unsigned long int i = 0; i < m_iD.size(); ++i )
   {
      if ( itor->iD().kernel()     == m_iD[i].kernel() &&
           itor->iD().model()      == m_iD[i].model() &&
           itor->iD().iD()         == m_iD[i].iD() &&
           itor->iD().resultType() == m_iD[i].resultType() )
      {
         return i;
      }
   }
   return -1;
}

void TouchstoneWrapper::setCategoriesMappingStatistics ()
{

   // Macro Porosity (volume %)
   m_iD.push_back( TcfSchema::ResultHeaderIdentificationType( "modal", 0, 0, 4 ) );
   
 	// Intergranular volume (volume %)
   m_iD.push_back( TcfSchema::ResultHeaderIdentificationType( "model", 1, 0, 0 ) );
   
   // Quartz (Volume %)
   m_iD.push_back( TcfSchema::ResultHeaderIdentificationType( "model", 1, 3, 0 ) );

   // Total (Core) Porosity (volume %)
   m_iD.push_back( TcfSchema::ResultHeaderIdentificationType( "model", 1, 5, 0 ) );

   // Micro Porosity (volume %)
   m_iD.push_back( TcfSchema::ResultHeaderIdentificationType( "model", 1, 5, 1 ) );

   // Permeability (md)
   m_iD.push_back( TcfSchema::ResultHeaderIdentificationType( "model", 1, 6, 0 ) );



   // fill m_statsVec with the appropriate StatisticsType values
   m_statsVect[stdev]=TcfSchema::StatisticsType::stdev;
   m_statsVect[mean]=TcfSchema::StatisticsType::mean;
   m_statsVect[geomean]=TcfSchema::StatisticsType::geomean;
   m_statsVect[skewness]=TcfSchema::StatisticsType::skewness;
   m_statsVect[kurtosis]=TcfSchema::StatisticsType::kurtosis;
   m_statsVect[minimum]=TcfSchema::StatisticsType::minimum ;
   m_statsVect[maximum]=TcfSchema::StatisticsType::maximum;
   m_statsVect[mode]=TcfSchema::StatisticsType::mode;
   m_statsVect[P1]=TcfSchema::StatisticsType::P1;
   m_statsVect[P5]=TcfSchema::StatisticsType::P5;
   m_statsVect[P10]=TcfSchema::StatisticsType::P10;
   m_statsVect[P15]=TcfSchema::StatisticsType::P15;
   m_statsVect[P20]=TcfSchema::StatisticsType::P20;
   m_statsVect[P25]=TcfSchema::StatisticsType::P25;
   m_statsVect[P30]=TcfSchema::StatisticsType::P30;
   m_statsVect[P35]=TcfSchema::StatisticsType::P35;
   m_statsVect[P40]=TcfSchema::StatisticsType::P40;
   m_statsVect[P45]=TcfSchema::StatisticsType::P45;
   m_statsVect[P50]=TcfSchema::StatisticsType::P50;
   m_statsVect[P55]=TcfSchema::StatisticsType::P55;
   m_statsVect[P60]=TcfSchema::StatisticsType::P60;
   m_statsVect[P65]=TcfSchema::StatisticsType::P65;
   m_statsVect[P70]=TcfSchema::StatisticsType::P70;
   m_statsVect[P75]=TcfSchema::StatisticsType::P75;
   m_statsVect[P80]=TcfSchema::StatisticsType::P80;
   m_statsVect[P85]=TcfSchema::StatisticsType::P85;
   m_statsVect[P90]=TcfSchema::StatisticsType::P90;
   m_statsVect[P95]=TcfSchema::StatisticsType::P95;
   m_statsVect[P99]=TcfSchema::StatisticsType::P99;
	
}


TouchstoneWrapper::TouchstoneWrapper( const char * burhistFile
                                    ,       char * filename
                                    , const char * results
                                    , const char * status
                                    , const char * rank
                                    , int          verboseLevel
                                    )
                                    : m_burhistFile( burhistFile )
                                    , m_filename( filename )
                                    , m_results( results )
                                    , m_status( status )
                                    , m_rank( rank )
                                    , m_verboseLevel( verboseLevel )
                                    , m_statsVect( statSize )
{

   char * geocosmDir = getenv ( "GEOCOSMDIR" );
   std::string workingDirstr( geocosmDir );
   try
   {

      Geocosm::TsLibPluginManager::LoadPluginLibraries( geocosmDir );

      // Initialize Touchstone library interface object.
      m_tslib = Geocosm::TsLibPluginManager::CreateTsLibPlugin();

      // Initialize Touchstone configuration file object.
      m_tcfInfo = Geocosm::TsLibPluginManager::CreateTcfInfo();
		
      // set up categories and statistics
      setCategoriesMappingStatistics ( );
   
      /** limit the size of core dump files **/
      m_coreSize.rlim_cur = 0;
      m_coreSize.rlim_max = 0;
      setrlimit(RLIMIT_CORE, &m_coreSize);   

   }
   catch ( const GeocosmException & g )
   {  
      throw Exception() << "TouchstoneWrapper() GeocosmException on LoadPluginLibraries( geocosmDir ) call " << g << " on MPI process " << m_rank;
   }
   catch (const std::runtime_error & r)
   {
      throw Exception() << "TouchstoneWrapper() std::runtime_error on LoadPluginLibraries() call: " << r.what() <<" on MPI process "<< m_rank;
   } 
   catch ( const std::exception & e )
   {
      throw Exception() << "TouchstoneWrapper() std::exception on LoadPluginLibraries() call: " << e.what() <<" on MPI process "<< m_rank;
   }
   catch (...)
   {
      throw Exception() << "TouchstoneWrapper unknown exception on LoadPluginLibraries( ) call on MPI process "<< m_rank;
   }  
   
}

TouchstoneWrapper::~TouchstoneWrapper ( ) 
{
   delete m_tcfInfo;
   delete m_tslib;
   
   ostringstream oss;
   
   try
   {
      Geocosm::TsLibPluginManager::UnloadPluginLibraries();
   }
   catch ( const GeocosmException   & g ) { oss << "~TouchstoneWrapper() GeocosmException on UnloadPluginLibraries() call:"    << g << "on MPI process " << m_rank; message( oss.str() ); }
   catch ( const std::runtime_error & r ) { oss << "~TouchstoneWrapper() std::runtime_error on UnloadPluginLibraries() call: " << r.what() << "on MPI process " << m_rank; message( oss.str() ); } 
   catch ( const std::exception     & e ) { oss << "~TouchstoneWrapper() std::exception on UnloadPluginLibraries() call: "     << e.what() << "on MPI process " << m_rank; message( oss.str() ); }
   catch (...)                            { oss << "~TouchstoneWrapper ( ) unknown exception on UnloadPluginLibraries( ) call"; message( oss.str() ); }
	
}


bool TouchstoneWrapper::loadTcf ( ) 
{

   //load tcf file
   m_tcfInfo->Tcf( m_filename );

   //is a direct analog run
   m_directAnalogRun = m_tcfInfo->IsDirectAnalogRun();

   //create number of realizations  
   if ( !m_directAnalogRun ) m_nrOfRealizations = m_tcfInfo->Realizations();	

   // Create a Touchstone Calculation Context
   m_tslibCalcContext = m_tslib->CreateContext ( m_filename );

   //get a copy of the result headers
   TcfSchema::ResultHeadersType savedResultHeaders( m_tslibCalcContext->ResultHeaders() );

   //do something with the headers
   TcfSchema::DetailHeadersType::modalHeaders_iterator itor;

   //modAls
   int categoriesOrder = 0;
   TcfSchema::DetailHeadersType::modalHeaders_sequence modalHeaders = savedResultHeaders.detailedHeaders().get().modalHeaders();

   for ( itor  = savedResultHeaders.detailedHeaders().get().modalHeaders().begin();
         itor != savedResultHeaders.detailedHeaders().get().modalHeaders().end();
         itor++ )
   {
      int result = SaveResultHeader( itor );
      if ( result != -1 ) 
      {     
         itor->saved( 1 );
         m_categoriesMappingOrder[result] = categoriesOrder;        
         ++categoriesOrder;  
      }
      else
      {
         itor->saved( 0 );
      }
   }

   //modEls 
   TcfSchema::DetailHeadersType::modelHeaders_sequence modelHeaders = savedResultHeaders.detailedHeaders().get().modelHeaders();
   for ( itor  = savedResultHeaders.detailedHeaders().get().modelHeaders().begin();
         itor != savedResultHeaders.detailedHeaders().get().modelHeaders().end();
         itor++)
   {
      int result = SaveResultHeader( itor );
      if ( result != -1 ) 
      {   	
         itor->saved( 1 );      
         m_categoriesMappingOrder[result] = categoriesOrder;
         ++categoriesOrder;
      }
      else
      {
         itor->saved( 0 );
      }
   }

   //at this point the user has selected which results to save... so update m_tslib with that list
   //note this list should have all the same entries as the list returned above, just with the 
   //saved field toggled.  Removing, reordering, adding, will have unintended consequences, and not
   //desired ones.
   m_tslibCalcContext->ResultHeaders( savedResultHeaders );

   // get the statistics that have been configured "on" from the tcf
   TcfSchema::StatisticsDefaultsType stats( m_tslibCalcContext->Statistics() );

   stats.statistics().clear();
   
   // fill stats.statistics()
   for(size_t i = 0 ; i != m_statsVect.size() ; ++ i) stats.statistics().push_back(m_statsVect[i]);
   
   //add or remove to the stats list, then update m_tslib...
   m_tslibCalcContext->Statistics( stats );

   //now that m_tslib has been updated with the list of headers that should be saved		
   //we ask m_tslib for the complete list of "statistics" headers.
   //say the user selected categories 1, 5, and 10 be saved, and stats "mean, mode, and stdev" to
   //calculate.  You would then get result headers:
   //Cat 1 Mean, Cat 1 Mode, Cat 1 stdve, Cat 5 Mean, Cat 5 Mode, Cat 5 stdev, Cat 10 Mean, Cat 10 Mode, Cat 10 stdev ...
   
   try 
   { 
      int randomSeed = 0; 
      //read the seed parameter from the TCF file using the RealizationSeed() function
      randomSeed = m_tcfInfo->RealizationSeed();
      if ( !m_directAnalogRun ) m_tslibCalcContext->CreateRealizations( m_nrOfRealizations, randomSeed, 1); 
   }      
   catch ( const GeocosmException & g ) 
   {  
      throw Exception() << "loadTcf() GeocosmException on CreateRealization( ) call:" << g <<" on MPI process "<< m_rank;
   }
   catch ( const std::runtime_error & r )
   {
      throw Exception() << "loadTcf() std::runtime_error on CreateRealization( ) call: "<< r.what() <<" on MPI process "<< m_rank;
   } 
   catch ( const std::exception & e )
   {
      throw Exception() << "loadTcf() std::exception on CreateRealization( ) call: " << e.what() <<" on MPI process "<< m_rank;
   }
   catch (...)
   {
      throw Exception() << "loadTcf() unknown exception on CreateRealization( ) call, on MPI process "<< m_rank;
   }
    
   return true;
   
}

void TouchstoneWrapper::calculateWrite ( ) {
   try
   {
      // initialize indexes
      int firstI			= -1;
      int lastI 			= -1;
      int firstJ 			= -1;
      int lastJ 			= -1;
      int numLayers		= -1;
      int numActive		= -1;
      int iD 				= -1;
      std::vector<size_t> usedSnapshotsIndexes;
   
      mkfifo(m_status, 0777);
      int fd = open(m_status, O_WRONLY| O_NONBLOCK);
   
      ReadBurial ReadBurial(m_burhistFile);
      ReadBurial.readIndexes(&firstI, &lastI, &firstJ, &lastJ, &numLayers, &numActive);
      ReadBurial.readSnapshotsIndexes(usedSnapshotsIndexes);
       
      TouchstoneFiles WriteTouchstone(m_results);
      WriteTouchstone.writeOrder(m_categoriesMappingOrder);  
      double fractionCompleted = 1e-6;
      int step = 0;
      
      // write a small number to tell the parent process that the calculation has started
      write( fd, &fractionCompleted, sizeof( fractionCompleted ) );


      for ( int l = 1; l <= numLayers; ++l )
      {
         for ( int i = firstI; i <= lastI; ++i )
         {
            for( int j = firstJ; j <= lastJ; ++j )
            {	
               size_t numTimeSteps = 0;
            
               ReadBurial.readNumTimeStepsID(&numTimeSteps, &iD);

               if ( numActive > 0 && numTimeSteps > 0) 
               {
                  std::vector<Geocosm::TsLib::burHistTimestep> burHistTimesteps(numTimeSteps) ; 
                  
                  // determine how many active time steps in the burial history. this is done in the reading of the file
                  step += ReadBurial.readBurialHistory(burHistTimesteps,numTimeSteps); 
                  
                  m_tslibBurialHistoryInfo.burialHistoryTSteps = &burHistTimesteps[0];
                  m_tslibBurialHistoryInfo.count               = numTimeSteps;
                  m_tslibBurialHistoryInfo.iD                  = iD; 

                  m_tslibCalcContext->Calculate( m_tslibBurialHistoryInfo, true ); 

                  WriteTouchstone.writeNumTimeSteps(numTimeSteps);  
       
                  for( size_t sn = 0; sn < usedSnapshotsIndexes.size(); ++sn )
                  {
                     writeTouchstoneResults( numTimeSteps - usedSnapshotsIndexes[sn] - 1, WriteTouchstone );
                  }

                  fractionCompleted =  (double) step / (double) numActive ;
                  
               } 
               else 
               {
                  WriteTouchstone.writeNumTimeSteps(numTimeSteps);
				      if (numActive == 0) fractionCompleted = 1.0;
               }
               
               if ( write( fd, &fractionCompleted, sizeof( fractionCompleted ) ) < 0 )
               {
                  ostringstream oss;
                  oss << "Could not write the status file on calculateWrite(), error code " << std::strerror( errno ) <<" on MPI process "<< m_rank;
                  message( oss.str() );
               }                          
            }  
         }
      }
   }
   catch ( const GeocosmException & g )
   {  
      ostringstream oss;
      oss << "calculateWrite() GeocosmException " << g <<" on MPI process "<< m_rank;
      throw Exception() << oss;
   }
   catch ( const std::runtime_error & r )
   {
      throw Exception() << "calculateWrite() std::runtime_error " << r.what() <<" on MPI process "<< m_rank;
   } 
   catch ( const std::exception & e )
   {
      throw Exception() << "calculateWrite() std::exception " << e.what() <<" on MPI process "<< m_rank;
   }
   catch (...)
   {
      throw Exception() << "calculateWrite( ) unknown exception on MPI process "<< m_rank;
   }
}

void TouchstoneWrapper::writeTouchstoneResults(int timestepIndex, TouchstoneFiles& WriteTouchstone) {

   GeoKernelDetailResultData detailedResults;

   std::vector<double> outputProperties( numberOfTouchstoneProperties * numberOfStatisticalOutputs, 99999.0);
   std::vector<double> actualOutputProperties(numberOfTouchstoneProperties * numberOfStatisticalOutputs, 99999.0);

   // Get statstics results from touchstone library;  
   if( m_tslibCalcContext->GetPredictionStatisticsResults( timestepIndex, detailedResults) )
   {     
      // Get the output from the touchstone
      int counter = 0;

		//Intergranular volume is always the first saved result
      for (unsigned int ii = 0; ii < detailedResults.modalResults.count; ++ii)
      {
         actualOutputProperties [ ii + counter ] =  detailedResults.modalResults.results[ ii ];
      }

      counter = detailedResults.modalResults.count;
      for (unsigned int ii = 0; ii < detailedResults.modelResults.count; ++ii)
      {
         actualOutputProperties [ ii + counter ] =  detailedResults.modelResults.results[ ii ];
      }

      counter = 0;
      // If saved, the last model result is permeability
      int permeability = m_iD.size() - 1; 
      for ( int ii = 0; ii < numberOfTouchstoneProperties - 1; ++ii )
      {
         for ( int jj = 0; jj < numberOfStatisticalOutputs - 1; ++jj )
         {
            if (ii != permeability && jj == geomean) continue; 
            {            	
            outputProperties [  ii * numberOfStatisticalOutputs + jj ] = actualOutputProperties [  counter ];
            counter += 1;
            }
           ;
         }
      }    		
   }
   else
   {
      if (timestepIndex > -1) throw Exception() << "writeTouchstoneResults() Failed to get statistics results for all categories on MPI process "<< m_rank;
   }

   for(int ii = 0; ii < numberOfTouchstoneProperties * numberOfStatisticalOutputs; ++ii)
   {
      actualOutputProperties [ ii ] = 99999.0;
   }

   // Get raw results from touchstone library;  
   if( m_tslibCalcContext->GetPredictionRawResults( timestepIndex, detailedResults) )
   {   
      // Get the output from the touchstone
      int counter = 0;

      for (unsigned int ii = 0; ii < detailedResults.modalResults.count; ++ii)
      {
         actualOutputProperties [ ii + counter ] =  detailedResults.modalResults.results[ ii ];
         //cout <<  actualOutputProperties [ ii + counter ] << endl;
      }

      counter = detailedResults.modalResults.count;
      for (unsigned int ii = 0; ii < detailedResults.modelResults.count; ++ii)
      {
         actualOutputProperties [ ii + counter ] =  detailedResults.modelResults.results[ ii ];
         //cout <<  actualOutputProperties [ ii + counter ] << endl;
      }
      for ( int ii = 0; ii < numberOfTouchstoneProperties - 1; ++ii )
      {
         outputProperties [  (ii + 1) * numberOfStatisticalOutputs - 1  ] = actualOutputProperties [  ii ];
      }
   }
   else
   {
      if (timestepIndex > -1) throw Exception() << "writeTouchstoneResults() Failed to get raw results for all categories on MPI process "<< m_rank;
   }

   // Fill the m_tslib actual output results
   for ( int jj = 0; jj <  numberOfStatisticalOutputs; ++jj )
   {
      outputProperties [ 6 * numberOfStatisticalOutputs + jj ] = std::log( outputProperties [ 4 * numberOfStatisticalOutputs + jj ] );
   }
   
   // Write results
   try
   {
      WriteTouchstone.writeArray( outputProperties );
   }
   catch ( const std::runtime_error & r )
   {
      throw Exception() << "writeTouchstoneResults() std::runtime_error " << r.what() <<" on MPI process "<< m_rank;
   } 

}
