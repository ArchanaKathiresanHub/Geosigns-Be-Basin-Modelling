// TsLibCppTester.cpp : Defines the entry point for the console application.
//
#include <mpi.h>

#include <stdlib.h>
#include <stdio.h>

#include <geocosmexception.h>

#include <sstream>
#include <iostream>
#include <vector>


#include "tslibloader.h"
#include "codetimer.h"


using namespace Geocosm;
using namespace std;


#ifdef _WIN32
   #include <direct.h>
   #define GetCurrentDir _getcwd
   #define DIRSEPARATOR "\\"
#else
   #include <unistd.h>
   #define GetCurrentDir getcwd
   #define DIRSEPARATOR "/"
   #include <ftw.h>
#endif

int mpiGetRank()
{
   static int rank = -1;
   if ( rank < 0 )
   {
      MPI_Comm_rank( MPI_COMM_WORLD, &rank );
   }
   return rank;
}

int mpiGetSize()
{
   static int size = -1;
   if ( size < 0 )
   {
      MPI_Comm_size( MPI_COMM_WORLD, &size );
   }
   return size;
}

//////////////////////////////////////////////////////////////////
// Set of aux. functions to set/clean up temporary folder to keep Matlab MCR files during CreateRealization call
// it could be removed if there will be the only one call for CreateRealization()
static void SetUpTempMCRFolder()
{
   std::stringstream oss;
#ifdef _WIN32
   const char * workingDir = GetCurrentDir( NULL, 0 );
   oss << "MCR_CACHE_ROOT=" << workingDir << DIRDELIMCHAR << "mcrTemp_" << mpiGetRank();
#else
   const char * workingDir = "/tmp/TsLibDemo_";
   oss << "MCR_CACHE_ROOT=" << workingDir << getpid();
#endif

   putenv( strdup( oss.str().c_str() ) );

#ifdef _WIN32
   free( workingDir );
#endif
}

#ifndef _WIN32
static int unlink_cb( const char * fpath, const struct stat * sb, int typeflag, struct FTW *ftwbuf )
{
    int rv = remove(fpath);
    if ( rv ) perror( fpath );
    return rv;
}
#endif

static int CleanTempMCRFolder()
{
   const char * path = getenv( "MCR_CACHE_ROOT" );
#ifndef _WIN32
   return nftw( path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS );
#endif
}

//////////////////////////////////////////////////////////////////

void writeResultsToFile( Geocosm::TsLib::CalcContextInterface* tslibCalcContext, int part = 0 )
{
   cout << "Writing results to file..." << endl;

   bool more;
	unsigned int index = 0;
	
   GeoKernelDetailResultData detailedResults;
	
   std::stringstream ofn;
	ofn << "./statisticsResults" << part << ".txt";

	FILE * fp = fopen( ofn.str().c_str(), "wt" );
	if ( fp != NULL )
   {
		while ((more = tslibCalcContext->GetPredictionStatisticsResults( index, detailedResults)))
		{
			fprintf( fp,"%d,%d,",detailedResults.burialHistoryID, detailedResults.tstep );
			for ( unsigned int i = 0; i < detailedResults.burialHistoryResults.count; i++ )
			{	
				fprintf( fp,"%.8f,",detailedResults.burialHistoryResults.results[i] );
			}
			for ( unsigned int i = 0; i < detailedResults.modalResults.count; i++ )
			{
				fprintf( fp,"%.8f,",detailedResults.modalResults.results[i] );
			}
			for ( unsigned int i = 0; i < detailedResults.modelResults.count; i++ )
			{
				fprintf(fp,"%.8f,",detailedResults.modelResults.results[i]);
			}
			fprintf( fp,"\n" );
			index++;
		}
		fclose( fp );
	   cout << "Results written to file." << endl;
	}
}

///////////////////////////////////////////////////////////////////////////////
// The implementation is simplistic 
///////////////////////////////////////////////////////////////////////////////
void RunTest( Geocosm::TsLib::CalcContextInterface* tslib )
{
   bool ranOK = false;
   CodeTimer ct;

	cout << endl << "Tslib tests..." << endl;
   cout << "Starting calculations..." << endl;  
   ct.Start();

   try
   {
   	tslib->Calculate();//normally you would pass in burial histories here.
      cout << " Calculate Done" << endl;
      ranOK = true;
   }
   catch ( std::exception& e )
   {
      cout << " Calculate Failed" << endl;
      cout << e.what() << endl;
   }

   ct.Stop();

   cout << "Processing time:" << ct.GetElapseTimeMS() << endl;

	if ( ranOK )
   {
	   writeResultsToFile( tslib, mpiGetRank() );
	}
}

void TestTcfInfo( const std::string & workingDir )
{
	try {
		std::auto_ptr<Geocosm::TsLib::TcfInfoInterface> tcfInfo (Geocosm::TsLibPluginManager::CreateTcfInfo());		
		tcfInfo->Tcf(workingDir + "/sampletestdata/RQCDataSet.tcf");
		cout << "Tcf Tests..." << endl;
		cout << "RQCDataSet.tcf: Is Version 1 Tcf: " << tcfInfo->IsV1() << " Is Version 2 Tcf: " << tcfInfo->IsV2() << endl;
		if (tcfInfo->IsV2()) {
			//do something with the header..
			cout << "Run Name: " << tcfInfo->Header().runName() << endl;
			cout << "Realizations in Tcf: " << tcfInfo->Realizations() << endl;
			cout << "Is Direct Analog Run: " << tcfInfo->IsDirectAnalogRun() << endl;
		}
		tcfInfo->Tcf(workingDir + "/sampletestdata/RQCDataSet-DirectAnalog.tcf");
		cout << "RQCDataSet - DirectAnalog.tcf: Is Version 1 Tcf: " << tcfInfo->IsV1() << " Is Version 2 Tcf: " << tcfInfo->IsV2() << endl;
		if (tcfInfo->IsV2()) {
			//do something with the header..
			cout << "Run Name: " << tcfInfo->Header().runName() << endl;
			cout << "Realizations in Tcf: " << tcfInfo->Realizations() << endl;
			cout << "Is Direct Analog Run: " << tcfInfo->IsDirectAnalogRun() << endl;
		}
		tcfInfo->Tcf(workingDir + "/sampletestdata/shell505.tcf");
		cout << "shell505.tcf: Is Version 1 Tcf: " << tcfInfo->IsV1() << " Is Version 2 Tcf: " << tcfInfo->IsV2() << endl;
		// no header info for version 1

		//now try a non TCF file, but valid
		tcfInfo->Tcf(workingDir + "/sampletestdata/tslibheader.xml");
		cout << "tslibheader.xml: Is Version 1 Tcf: " << tcfInfo->IsV1() << " Is Version 2 Tcf: " << tcfInfo->IsV2() << endl;

		//now try a bad file, does not exist 
		tcfInfo->Tcf("blah.txt");
		//shouldn't get to next line, as an exception will be thrown
		cout << "blah.txt: Is Version 1 Tcf: " << tcfInfo->IsV1() << " Is Version 2 Tcf: " << tcfInfo->IsV2() << endl;

	}
	catch ( Geocosm::GeocosmException & ex )
	{
		cout << ex << endl;
	}
}

int main( int argc, char* argv[] )
{

   if ( argc < 3 )
   {
      cout << "Usage: " << argv[0] << " <1 for tcf tests> <number of realisations>" << endl;
      return( 0 );
   }
      
   int dotests              = atol( argv[1] );
   int numberOfRealisations = atol( argv[2] );

   if ( numberOfRealisations < 10 || numberOfRealisations > 5000 ) {
      cout << "Invalid realization count specified: " << numberOfRealisations << ". Must be from 10 till 5000." << endl;
      return -1;
   }

   MPI_Init( &argc, &argv );

   // set up /tmp/TsLibDemo_<pid> temp folder for each MPI process to keep MCR cache files
   SetUpTempMCRFolder();

   char * pluginDir  = getenv( "GEOCOSMDIR" );
	char * workingDir = GetCurrentDir( NULL, 0 );

	cout << "TsLibCppTester starting... loading tslib..." << endl << pluginDir << endl;

	//test delayed load, at this point you should be able to start this program 
	//without geocosmxmllibbasecpp.dll on the path (or renamed to something else)
	//now rename it back and continue, it will be automatically loaded by the
	//windows loader when it is needed.  
   try {
#ifdef _WIN32
		LibLoader::AddToLibSearchPath( pluginDirDir );
#endif
		//on Linux only option is rpath during link or LD_LIBRARY_PATH evn var

		//we don't delay load tslib.dll via the project settings, instead we explicitly delay load it
		//using LoadLibrary/GetProcAdrress.
		Geocosm::TsLibPluginManager::LoadPluginLibraries( pluginDir ); //this path must be the path with tslib.dll in it, even if rpath and SetDllDirectory are used
      cout << "\nsuccess...\n" << endl;

		if ( dotests == 1 ) {
			TestTcfInfo( workingDir );
		}

		//get me a TsLibInterface object...
		std::auto_ptr<Geocosm::TsLib::TsLibInterface> tslib( Geocosm::TsLibPluginManager::CreateTsLibPlugin() );

		//verify the loaded plugin is for the same version that we compiled against.
		if ( tslib->GetInterfaceVersion() != Geocosm::TsLib::TSLIB_INTERFACE_VERSION ) {
			//mismatch
			throw GeocosmException("TsLib Interface mismatch");
		}

      std::auto_ptr<Geocosm::TsLib::CalcContextInterface> tslibCalcContext;
     
#ifdef AsItShouldBe
      if ( mpiGetRank() == 0 ) // master process
      {
         // create a calc context, this creates a master context as the tcf is passed in.
   		tslibCalcContext.reset( tslib->CreateContext( string( workingDir ) + "/sampletestdata/RQCDataSet.tcf" ) );

         cout << "Creating realizations " << numberOfRealisations << " ..." << endl;
         tslibCalcContext->CreateRealizations( numberOfRealisations );
         cout << "Finished creating realizations." << endl;

         // serialise context to something simple - string or array
         const string & cntxStr = tslibCalcContext->CloneToString();
         int cntxStrSize = cntxStrSize.size() + 1;

         // send to each slave process serialized to string context
         for ( int p = 1; p < mpiGetSize(); ++p )
         {
            MPI_Send( &cntxStrSize,       1,           MPI_INT,  p, MSIZE, MPI_COMM_WORLD ); // sending string size
            MPI_Send( &(cntxStr.c_str()), cntxStrSize, MPI_BYTE, p, MSEND, MPI_COMM_WORLD ); // sending string itself
         }            
      }
      else
      {
         // get in slave from master serialized context and recreate it from string
         int cntxStrSize = 0;
         MPI_Recv( &cntxStrSize, 1, MPI_INT, 0, MSIZE, MPI_COMM_WORLD ); // get string length

         // allocate buffer for string, length alredy includes trailing zero but just for safety ask for one more byte in buffer
         char * str = new char[cntxStrSize+1];
         MPI_Recv( &str, cntxStrSize, MPI_BYTE, 0, MSEND, MPI_COMM_WORLD );  // get string itself
         tslibCalcContext = tslib->CreateContextFromString( string( str ) ); // recreate Context
         delete [] str;
      }
#else // as it is now

         // create a calc context, this creates a master context as the tcf is passed in.
         tslibCalcContext.reset( tslib->CreateContext( string( workingDir ) + "/sampletestdata/RQCDataSet.tcf" ) );
   
         cout << "Creating realizations " << numberOfRealisations << " ..." << endl;
         tslibCalcContext->CreateRealizations( numberOfRealisations );
         cout << "Finished creating realizations." << endl;
#endif

      // get a copy of the result headers
		TcfSchema::ResultHeadersType savedResultHeaders( tslibCalcContext->ResultHeaders() );

		//do something with the headers
		TcfSchema::DetailHeadersType::modalHeaders_const_iterator itor;

		//modAls
		for (itor = savedResultHeaders.detailedHeaders().get().modalHeaders().begin();
			 itor != savedResultHeaders.detailedHeaders().get().modalHeaders().end();
			 itor++)
		{
			cout << (*itor) << endl;
		}
		//modEls 
		for (itor = savedResultHeaders.detailedHeaders().get().modelHeaders().begin();
			 itor != savedResultHeaders.detailedHeaders().get().modelHeaders().end();
			 itor++)
		{
			cout << (*itor) << endl;
		}
		//at this point the user has selected which results to save... so update tslib with that list
		//note this list should have all the same entries as the list returned above, just with the 
		//saved field toggled.  Removing, reordering, adding, will have unintended consequences, and not
		//desired ones.
		tslibCalcContext->ResultHeaders( savedResultHeaders );

		//get the list of all the statistics
		cout << "\nStatistic Info" << endl;
		std::vector<TcfSchema::StatisticsType>::const_iterator statItor;
		for (statItor = tslib->Statistics().begin();
			 statItor != tslib->Statistics().end();
			 statItor++)
		{
			cout << "Statistic: " << (*statItor) << endl;
		}

		// get the statistics that have been configured "on" from the tcf
		TcfSchema::StatisticsDefaultsType stats(tslibCalcContext->Statistics());
		TcfSchema::StatisticsDefaultsType::statistics_const_iterator stor;
		for (stor = stats.statistics().begin();
			 stor != stats.statistics().end();
			 stor++)
		{
			cout << (*stor) << endl;
		}
		//add or remove to the stats list, then update tslib...
		tslibCalcContext->Statistics(stats);

		//now that tslib has been updated with the list of headers that should be saved
		//we ask tslib for the complete list of "statistics" headers.
		//say the user selected categories 1, 5, and 10 be saved, and stats "mean, mode, and stdev" to
		//calculate.  You would then get result headers:
		//Cat 1 Mean, Cat 1 Mode, Cat 1 stdve, Cat 5 Mean, Cat 5 Mode, Cat 5 stdev, Cat 10 Mean, Cat 10 Mode, Cat 10 stdev ...
		const TcfSchema::ResultHeadersType& statHeaders = tslibCalcContext->StatisticsResultHeaders();

		RunTest( tslibCalcContext.get() );
	}
	catch (GeocosmException& ex)
	{
		cout << ex << endl;
	}
	catch (std::exception& ex)
	{
		cout << ex.what() << endl;
	}
   
   MPI_Barrier( MPI_COMM_WORLD );
   MPI_Finalize();

   cout << "Unloading libraries..." << endl;
	//done with tslib
	Geocosm::TsLibPluginManager::UnloadPluginLibraries();
	
	free(workingDir);
   
   cout << "Cleaning MCR temp folder" << endl;
   CleanTempMCRFolder();

	return 0;
}

