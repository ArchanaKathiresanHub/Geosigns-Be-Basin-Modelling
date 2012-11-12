#include <unistd.h>

#include "petsc.h"

#include "MasterTouch.h"

// Touchstone Include files

#include "FastTouch.h"
using namespace fasttouch;
#include <geocosmexception.h>
using namespace Geocosm;

#include <sstream>
using namespace std;

#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/PropertyValue.h"


void InitializeTimeComplete ();
bool ReportTimeToComplete (int stepCompleted, int totalNumberOfSteps, int & afterSeconds);

using namespace DataAccess;
using namespace Interface;

#include <stdlib.h>

// initialise statics for user interface
char* iSd_str             = {"Summary Standard Deviation"};
char* iMean_str           = {"Summary Mean"};
char* iSkewness_str       = {"Summary Skewness"};
char* iKurtosis_str       = {"Summary Kurtosis"};
char* iMin_str            = {"Summary Minimum"};
char* iMax_str            = {"Summary Maximum"};
char* iMode_str           = {"Summary Mode"};
char* iPercentile_str     = {"Percentile"};
char* iDistribution_str   = {"Distribution"};
char* iCore_equiv_str     = {"Porosity Core Equivalent"};
char* iIntergranular_str  = {"Porosity Intergranular Volume"};
char* iMacro_str          = {"Porosity Macro"};
char* iMicro_str          = {"Porosity Micro"};
char* iAbsolute_str       = {"Permeability Absolute"};
char* iLog_str            = {"Permeability Log10"};
char* iCement_Quartz_str  = {"Cement Quartz"};

extern void ReportProgress (const string & message);
//const int MasterTouch::numberOfTouchstoneProperties = 6;

// PUBLIC METHODS
//
/// The job of the constructor is to open the ResQ library and initialise
/// the format and category string lists
MasterTouch::MasterTouch (FastTouch * fastTouch) : m_fastTouch (fastTouch)
{
    // open TS library version 7.x
    char* geocosmDir = getenv ( "GEOCOSMDIR" );
    
    std::string workingDirstr( geocosmDir );

    Geocosm::TsLibPluginManager::LoadPluginLibraries( geocosmDir );

    // Initialize Touchstone library interface object.
    tslib = Geocosm::TsLibPluginManager::CreateTsLibPlugin();
    
    // Initialize Touchstone configuration file object.
    tcfInfo = Geocosm::TsLibPluginManager::CreateTcfInfo();

    // open TS library version 6.x
    if ( ! TsLib_Open() )
    {
        touchstoneError ("MasterTouch");
    }

    // set up fixed string lists
    setFormatsMapping ();
    setCategoriesMapping ();

}

MasterTouch::~MasterTouch()
{
    delete tcfInfo;
    delete tslib;
    Geocosm::TsLibPluginManager::UnloadPluginLibraries( );
    TsLib_Close();
}

/** The run function is responsible for carrying out the functional
 *  steps involved in using the ResQ library.
 *  So firstly, a request is sent to the ResQ library to open all the user-selected
 *  Tcf files. Then for each Tcf file, all calculations and saving of results
 *  are executed through the functions: calculate and saveGrids. Lastly, the 
 *  Tcf files are closed and dynamic memory is cleaned up.
 */
bool MasterTouch::run (void)
{
    // make sure data has been selected 
    if (m_fileList.size () < 1)
    {
        return false;
    }

    FileLayerCategoryMapInfoList::iterator it;

    for (it = m_fileList.begin (); it != m_fileList.end (); ++it)
    {
        const string & filename = (it->first);
        char * tcf_filename = (char *) filename.c_str ();

        // Touchstone Library Plugin Implementation
        
        //verify the loaded plugin is for the same version that we compiled against.
        if (tslib->GetInterfaceVersion() != Geocosm::TsLib::TSLIB_INTERFACE_VERSION ) 
        {
            //mismatch
            throw GeocosmException ( "TsLib Interface mismatch" );
        }
  
        tcfInfo->Tcf ( filename );
        
        if ( tcfInfo->IsV2() )
        {
            m_directAnalogRun = tcfInfo->IsDirectAnalogRun();
            
            string starting;

            if(!m_directAnalogRun) 
            {
                m_nrOfRealizations = tcfInfo->Realizations();
                stringstream ss; ss << m_nrOfRealizations;
                starting = "Loaded TsLib 7.0 Configuration file \"" 
                         + filename + "\" and setting up for a monte carlo calculation with "
                         + ss.str() +" realizations"; 
            }
            else
            {
                starting = "Loaded TsLib 7.0 Configuration file \""
                         + filename + "\" and setting up  for a direct analog calculation";
            }
            
            ReportProgress ( starting );
            // set up timestep boundaries
    
            // for each TCF file
            LayerCategoryMapInfoList::iterator outIt;
     
            m_layerList = &(m_fileList[filename]);
    
            for (outIt = m_layerList->begin (); outIt != m_layerList->end (); ++outIt)
            {
                // set current layer iterator so can retrieve all output 
                // combinations during write/save maps
                m_currentOutputs = (*outIt).second;
                string progressString;
   
                progressString = "Starting TCF: ";
                progressString += filename;
                progressString += ", Surface: ";
                progressString += (outIt->first).surface->getName ();
                progressString += ", Formation: ";
                progressString += (outIt->first).formation->getName ();
                ReportProgress (progressString);
                
                // execute TS calculate
    
                retrieveGridMaps ();
        
                bool calculated = calculate (tcf_filename, (outIt->first).surface, (outIt->first).formation);
            
                restoreGridMaps ();

                progressString = "Finished TCF: ";
                progressString += filename;
                progressString += ", Surface: ";
                progressString += (outIt->first).surface->getName ();
                progressString += ", Formation: ";
                progressString += (outIt->first).formation->getName ();
            
                ReportProgress (progressString);  

                if (!calculated) break;

            }
            // check for early exit from For loop
            if (outIt != m_layerList->end ())
            {
                break;
            }
        }
        else
        {
            if ( tcfInfo->IsV1() )
            {
                string starting = "Loaded TsLib 3.1.1 configuration file \"" + filename 
                                + "\" and setting up for a monte carlo calculation"; 
                ReportProgress ( starting );
                // load all touchstone files
                if (TsLib_LoadTcfs (&tcf_filename, 1) != 1)
                {
                    touchstoneError ("run");
                    return false;
                }
                
                // set up timestep boundaries

                // for each TCF file
                LayerCategoryMapInfoList::iterator outIt;
                
                m_layerList = &(m_fileList[filename]);
                for (outIt = m_layerList->begin (); outIt != m_layerList->end (); ++outIt)
                {
                    // set current layer iterator so can retrieve all output 
                    // combinations during write/save maps
                    m_currentOutputs = (*outIt).second;
                    string progressString;
                    
                    progressString = "Starting TCF: ";
                    progressString += filename;
                    progressString += ", Surface: ";
                    progressString += (outIt->first).surface->getName ();
                    progressString += ", Formation: ";
                    progressString += (outIt->first).formation->getName ();;
                    
                    ReportProgress (progressString);

                    // execute TS calculate
                    retrieveGridMaps ();
                    
                    bool calculated = calculateOld (tcf_filename, (outIt->first).surface, (outIt->first).formation);
                    
                    restoreGridMaps ();
                    progressString = "Finished TCF: ";
                    progressString += filename;
                    progressString += ", Surface: ";
                    progressString += (outIt->first).surface->getName ();
                    progressString += ", Formation: ";
                    progressString += (outIt->first).formation->getName ();
                    ReportProgress (progressString);
                    
                    if (!calculated) break;
                }
                
                // check for early exit from For loop
                if (outIt != m_layerList->end ())
                {
                    break;
                }
                
                TsLib_UnloadTcfs(&tcf_filename, 1);
                // clean up TS calculate memory
                TsLib_CalculationCleanUp ();
            }          
            else
            {
                cerr << filename << " is not a tcf file." << endl;
            }
        }
   }

   return true;
}

/** Each set of results requested by the user corresponds to a grid map
 *  generated by Cauldron, filled with ResQ calculation results.
 *  Each grid map created must correspond to a Tcf file, a specific layer,
 *  a results category and a result format.
 */
bool MasterTouch::addOutputFormat ( const string& filename, const Surface * surface, 
                                    const Formation * formation, const string & category,
                                    const string & format, short percent)
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

    const Snapshot * zeroSnapshot = m_fastTouch->findSnapshot (0);

    if (!zeroSnapshot)
    { 
        cerr << endl << "Could not create PropertyValue: " << propertyValueName  <<
        ", could not find present day snapshot"<< endl; 
        return false; 
    }

    PropertyValue * propertyValue = m_fastTouch->createMapPropertyValue ( propertyValueName, 
                                                                          zeroSnapshot, 0, 
                                                                          formation, surface); 
    if (!propertyValue) 
    { 
        cerr << endl << "Could not create PropertyValue named: " << propertyValueName << endl; 
        return false; 
    }
    
    GridMap * gridMap = propertyValue->getGridMap ();
 
    // create map info
    MapInfo map; 
    map.format = format; 
    map.gridMap = gridMap; 
    map.percent = percent;

    LayerInfo layer (surface, formation);

    // add layer info and map info to output list
    m_fileList[filename][layer][category].push_back(map);

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
bool MasterTouch::calculate ( const char *filename, const Surface * surface, 
                              const Formation * formation )
{
    int reportAfter = 30;
 
    // create burial history
    BurialHistory< Geocosm::TsLib::burHistTimestep >* burialHistory = new BurialHistory < Geocosm::TsLib::burHistTimestep > (surface, formation, m_fastTouch);
  
    // create progress bar
    int numTimeSteps;

    // for each defined node on reservoir surface  
    int firstI = m_fastTouch->getActivityOutputGrid ()->firstI ();
    int firstJ = m_fastTouch->getActivityOutputGrid ()->firstJ ();
    int lastI = m_fastTouch->getActivityOutputGrid ()->lastI ();
    int lastJ = m_fastTouch->getActivityOutputGrid ()->lastJ ();
    
    Geocosm::TsLib::burHistTimestep *burHistTimesteps;
   
    int step = 0;   
    int totalNumberOfSteps = (lastI + 1 - firstI) * (lastJ + 1 - firstJ);
    int maxNumTimeSteps = 0;
    bool allHaveCompleted = false;   

    // Create a Touchstone Calculation Context
    tslibCalcContext = tslib->CreateContext ( filename );
    
    //get a copy of the result headers
    TcfSchema::ResultHeadersType savedResultHeaders(tslibCalcContext->ResultHeaders());
    
    //do something with the headers
    TcfSchema::DetailHeadersType::modalHeaders_iterator itor;
    //modAls
    int categoriesOrder = 0;
    TcfSchema::DetailHeadersType::modalHeaders_sequence modalHeaders = savedResultHeaders.detailedHeaders().get().modalHeaders();
    for ( itor = savedResultHeaders.detailedHeaders().get().modalHeaders().begin();
          itor != savedResultHeaders.detailedHeaders().get().modalHeaders().end();
          itor++)
    {
        int result = SaveResultHeader (itor);
        if ( result != -1 ) 
        {
            (*itor).saved(1);
            m_categoriesMappingOrder[result] = categoriesOrder;
            ++categoriesOrder;
        }
        else
        {
            (*itor).saved(0);
        }
    }
    //modEls 
    TcfSchema::DetailHeadersType::modelHeaders_sequence modelHeaders = savedResultHeaders.detailedHeaders().get().modelHeaders();
    for ( itor = savedResultHeaders.detailedHeaders().get().modelHeaders().begin();
          itor != savedResultHeaders.detailedHeaders().get().modelHeaders().end();
          itor++)
    {
        int result = SaveResultHeader (itor);
        if ( result != -1 ) 
        {
            (*itor).saved(1);
            m_categoriesMappingOrder[result] = categoriesOrder;
            ++categoriesOrder;
        }
        else
        {
            (*itor).saved(0);
        }
    }
    m_categoriesMapping [iCore_equiv_str] = m_categoriesMappingOrder[0]; //TSLIB_RC_CORE_PORO;
    m_categoriesMapping [iIntergranular_str] = m_categoriesMappingOrder[1]; // TSLIB_RC_IGV;
    m_categoriesMapping [iMacro_str] = m_categoriesMappingOrder[2]; // TSLIB_RC_MACRO_PORO;
    m_categoriesMapping [iMicro_str] = m_categoriesMappingOrder[3]; //TSLIB_RC_MICRO_PORO;
    m_categoriesMapping [iAbsolute_str] = m_categoriesMappingOrder[4]; //TSLIB_RC_PERM;
    m_categoriesMapping [iCement_Quartz_str] = m_categoriesMappingOrder[5]; //TSLIB_RC_CMT_QRTZ;
    
    //at this point the user has selected which results to save... so update tslib with that list
    //note this list should have all the same entries as the list returned above, just with the 
    //saved field toggled.  Removing, reordering, adding, will have unintended consequences, and not
    //desired ones.
    tslibCalcContext->ResultHeaders(savedResultHeaders);
    
    // get the statistics that have been configured "on" from the tcf
    TcfSchema::StatisticsDefaultsType stats(tslibCalcContext->Statistics());
    
    stats.statistics().clear();

    stats.statistics().push_back(TcfSchema::StatisticsType::stdev);
    stats.statistics().push_back(TcfSchema::StatisticsType::mean);
    stats.statistics().push_back(TcfSchema::StatisticsType::skewness);
    stats.statistics().push_back(TcfSchema::StatisticsType::kurtosis);
    stats.statistics().push_back(TcfSchema::StatisticsType::minimum);
    stats.statistics().push_back(TcfSchema::StatisticsType::maximum);
    stats.statistics().push_back(TcfSchema::StatisticsType::mode);
    stats.statistics().push_back(TcfSchema::StatisticsType::P1);
    stats.statistics().push_back(TcfSchema::StatisticsType::P5);
    stats.statistics().push_back(TcfSchema::StatisticsType::P10);
    stats.statistics().push_back(TcfSchema::StatisticsType::P15);
    stats.statistics().push_back(TcfSchema::StatisticsType::P20);
    stats.statistics().push_back(TcfSchema::StatisticsType::P25);
    stats.statistics().push_back(TcfSchema::StatisticsType::P30);
    stats.statistics().push_back(TcfSchema::StatisticsType::P35);
    stats.statistics().push_back(TcfSchema::StatisticsType::P40);
    stats.statistics().push_back(TcfSchema::StatisticsType::P45);
    stats.statistics().push_back(TcfSchema::StatisticsType::P50);
    stats.statistics().push_back(TcfSchema::StatisticsType::P55);
    stats.statistics().push_back(TcfSchema::StatisticsType::P60);
    stats.statistics().push_back(TcfSchema::StatisticsType::P65);
    stats.statistics().push_back(TcfSchema::StatisticsType::P70);
    stats.statistics().push_back(TcfSchema::StatisticsType::P75);
    stats.statistics().push_back(TcfSchema::StatisticsType::P80);
    stats.statistics().push_back(TcfSchema::StatisticsType::P85);
    stats.statistics().push_back(TcfSchema::StatisticsType::P90);
    stats.statistics().push_back(TcfSchema::StatisticsType::P95);
    stats.statistics().push_back(TcfSchema::StatisticsType::P99);
    //add or remove to the stats list, then update tslib...
    tslibCalcContext->Statistics(stats);

    //now that tslib has been updated with the list of headers that should be saved		
    //we ask tslib for the complete list of "statistics" headers.
    //say the user selected categories 1, 5, and 10 be saved, and stats "mean, mode, and stdev" to
    //calculate.  You would then get result headers:
    //Cat 1 Mean, Cat 1 Mode, Cat 1 stdve, Cat 5 Mean, Cat 5 Mode, Cat 5 stdev, Cat 10 Mean, Cat 10 Mode, Cat 10 stdev ...

    const TcfSchema::ResultHeadersType& statHeaders = tslibCalcContext->StatisticsResultHeaders();
    
    if(!m_directAnalogRun) tslibCalcContext->CreateRealizations(m_nrOfRealizations);
    
    InitializeTimeComplete ();
    for (int i = firstI; i <= lastI; ++i)
    {
        int numJ = 0;
        for (int j = firstJ; j <= lastJ; ++j)
        {
            // extract burial history 
            if ((burHistTimesteps = burialHistory->returnAsArray (i, j, numTimeSteps, true)) != NULL)
            {
                tslibBurialHistoryInfo.burialHistoryTSteps = burHistTimesteps;
                tslibBurialHistoryInfo.count               = numTimeSteps;
                tslibBurialHistoryInfo.iD                  = (i - firstI)  * ( lastJ - firstJ + 1) + j;

                maxNumTimeSteps = maxNumTimeSteps < numTimeSteps ? numTimeSteps : maxNumTimeSteps;
                ++numJ;

                 tslibCalcContext->Calculate( tslibBurialHistoryInfo, true ); 
                 writeResultsToGrids (i, j, numTimeSteps - 1);
            }
            allHaveCompleted = ReportTimeToComplete (++step, totalNumberOfSteps, reportAfter);
        }
    }

    while (!allHaveCompleted)
    {
        sleep (5);
        allHaveCompleted = ReportTimeToComplete (totalNumberOfSteps, totalNumberOfSteps, reportAfter);
    }
    MPI_Barrier (PETSC_COMM_WORLD);
    delete burialHistory;
   return true;
}

/** This function controls the build of the Cauldron - Old ResQ functionality. 
 *  Firstly, it creates a burial history for the user-chosen layer through
 *  the Burial History class. THen for each grid point in the layer, it called th
 *  ResQ::calculate function, passing it the burial history data for that particular
 *  coordinate and the Tcf file associated with that layer, to generate all ResQ
 *  results for that particular coordinate. Finally it writes the results to a result
 *  list which is saved to file at a later stage.
 */
bool MasterTouch::calculateOld ( const char *filename, const Surface * surface, 
                              const Formation * formation )
{
    int reportAfter = 30;

    // create burial history
    BurialHistory<tsLib_burHistTimestep> * burialHistory = new BurialHistory <tsLib_burHistTimestep> (surface, formation, m_fastTouch);
 
    // create progress bar
    int numTimeSteps;

    // for each defined node on reservoir surface  
    int firstI = m_fastTouch->getActivityOutputGrid ()->firstI ();
    int firstJ = m_fastTouch->getActivityOutputGrid ()->firstJ ();
    int lastI = m_fastTouch->getActivityOutputGrid ()->lastI ();
    int lastJ = m_fastTouch->getActivityOutputGrid ()->lastJ ();
 
    tsLib_burHistTimestep* burHistTimesteps;
 
    int step = 0;
    int totalNumberOfSteps = (lastI + 1 - firstI) * (lastJ + 1 - firstJ);
    int maxNumTimeSteps = 0;
 
    InitializeTimeComplete ();
    bool allHaveCompleted = false;

    for (int i = firstI; i <= lastI; ++i)
    {
        int numJ = 0;
        for (int j = firstJ; j <= lastJ; ++j)
        {
            // extract burial history 
            if ((burHistTimesteps = burialHistory->returnAsArray (i, j, numTimeSteps, false)) != NULL)
            {
                maxNumTimeSteps = maxNumTimeSteps < numTimeSteps ? numTimeSteps : maxNumTimeSteps;     
                ++numJ;
                // call TS calculate with TCF filename and burial history
                if (TsLib_Calculate (const_cast < char *>(filename), burHistTimesteps, numTimeSteps) != 1)
                {
                    touchstoneError ("calculate");
                    return false;
                }
                // retrieve and write calculation results to grid
                writeResultsToGridsOld (i, j, numTimeSteps - 1);
            }
            allHaveCompleted = ReportTimeToComplete (++step, totalNumberOfSteps, reportAfter);
        }
    }

    while (!allHaveCompleted)
    {
        sleep (5);
        allHaveCompleted = ReportTimeToComplete (totalNumberOfSteps, totalNumberOfSteps, reportAfter);
    }
 
    MPI_Barrier (PETSC_COMM_WORLD);
 
    delete burialHistory;
    return true;
}


/** For each layer and associated tcf file, there may be more than one result
 *  category and format selected. This means that after the ResQ results are calculated
 *  for a grid point on a certain layer, all the categories and formats associated 
 *  with that particular layer should be extracted before the next grid point's 
 *  results are processed
 */
void MasterTouch::writeResultsToGrids (int i, int j, int timestepIndex)
{
    GeoKernelDetailResultData detailedResults;
    for(int ii = 0; ii < numberOfTouchstoneProperties * numberOfStatisticalOutputs; ++ii)
    {
        m_OutputProperties [ ii ] = 99999;
        m_ActualOutputProperties [ ii ] = 0;
    }

    // Get statstics results from touchstone library;  
    if( tslibCalcContext->GetPredictionStatisticsResults( timestepIndex, detailedResults) )
    {     
        // Get the output from the touchstone
        int counter = 0;
            
        for (unsigned int ii = 0; ii < detailedResults.modalResults.count; ++ii)
        {
            m_ActualOutputProperties [ ii + counter ] =  detailedResults.modalResults.results[ ii ];
        }

        counter = detailedResults.modalResults.count;
        for (unsigned int ii = 0; ii < detailedResults.modelResults.count; ++ii)
        {
             m_ActualOutputProperties [ ii + counter ] =  detailedResults.modelResults.results[ ii ];
        }
        for ( int ii = 0; ii < numberOfTouchstoneProperties - 1; ++ii )
        {
            for ( int jj = 0; jj < numberOfStatisticalOutputs - 1; ++jj )
            {
                m_OutputProperties [  ii * numberOfStatisticalOutputs + jj ] =  
                    m_ActualOutputProperties [  ii * ( numberOfStatisticalOutputs - 1 ) + jj ];
            }
        }
    }
    else
    {
        cerr << "Touch stone error: Failed to get statistics results for all categories." << endl;
    }
    
    for(int ii = 0; ii < numberOfTouchstoneProperties * numberOfStatisticalOutputs; ++ii)
    {
        m_ActualOutputProperties [ ii ] = 0;
    }

    // Get raw results from touchstone library;  
    if( tslibCalcContext->GetPredictionRawResults( timestepIndex, detailedResults) )
    {   
        // Get the output from the touchstone
        int counter = 0;

        for (unsigned int ii = 0; ii < detailedResults.modalResults.count; ++ii)
        {
            m_ActualOutputProperties [ ii + counter ] =  detailedResults.modalResults.results[ ii ];
            //cout <<  m_ActualOutputProperties [ ii + counter ] << endl;
        }

        counter = detailedResults.modalResults.count;
        for (unsigned int ii = 0; ii < detailedResults.modelResults.count; ++ii)
        {
             m_ActualOutputProperties [ ii + counter ] =  detailedResults.modelResults.results[ ii ];
            //cout <<  m_ActualOutputProperties [ ii + counter ] << endl;
        }
        for ( int ii = 0; ii < numberOfTouchstoneProperties - 1; ++ii )
        {
            {
                m_OutputProperties [  (ii + 1) * numberOfStatisticalOutputs - 1  ] =  
                    m_ActualOutputProperties [  ii ];
            }
        }
    }
    else
    {
        cerr << "Touch stone error: Failed to get raw results for all categories." << endl;
    }

    // Fill the tslib actual output results
    for ( int jj = 0; jj <  numberOfStatisticalOutputs; ++jj )
    {
        m_OutputProperties [ 6 * numberOfStatisticalOutputs + jj ] =  
            log ( m_OutputProperties [ 4 * numberOfStatisticalOutputs + jj ]);
    }

#if 0
    for ( int ii = 0; ii < numberOfTouchstoneProperties; ++ii )
    {   
        cout << ii << " = [ ";
        for ( int jj = 0; jj < numberOfStatisticalOutputs; ++jj )
        {
            cout << m_OutputProperties [ ii * numberOfStatisticalOutputs + jj ] << "; ";
        }
        cout << " ] "<< endl;
    }
    sleep( 1 );
#endif

    // loop through each combination of category and format choices
    // for the current layer    
    short resultFormat;
    // for each category selected
    for ( CategoryMapInfoList::iterator it = m_currentOutputs.begin(); it != m_currentOutputs.end (); ++it )
    {
        short resultCat = m_categoriesMapping[(*it).first]; 
        // for each format selected
        for ( MapInfoList::iterator mIt = ((*it).second).begin(); mIt != ((*it).second).end(); ++mIt )
        {
            // write appropriate results to grid
            resultFormat = m_formatsMapping[(*mIt).format];
            // Explanation to formula below:
            // if resultFormat = {SD = 0, 1, 2, 3, 4, 5, MODE = 6} then resultStat = resultFormat.
            // if resultFormat = { MODE } then resultStat = 7, ... 27 
            // if resultFormat = { DISTRIBUTION } then resultStat = 28;
            int resultStat = resultFormat 
                           + int (resultFormat > MODE) * ( m_percentPercentileMapping[ (*mIt).percent ] ) 
                           + int (resultFormat > PERCENTILE ) * ( 20 - m_percentPercentileMapping [ (*mIt).percent ] );
            (*mIt).gridMap->setValue (i, j,  m_OutputProperties[ resultCat *  numberOfStatisticalOutputs + resultStat ]);
        }
    }
}

/** For each layer and associated tcf file, there may be more than one result
 *  category and format selected. This means that after the ResQ results are calculated
 *  for a grid point on a certain layer, all the categories and formats associated 
 *  with that particular layer should be extracted before the next grid point's 
 *  results are processed
 */
void MasterTouch::writeResultsToGridsOld (int i, int j, int timestepIndex)
{
    // loop through each combination of category and format choices
    // for the current layer
    short resultFormat;
   
    // for each category selected
    for ( CategoryMapInfoList::iterator it = m_currentOutputs.begin(); it != m_currentOutputs.end (); ++it )
    {
        short resultCat = m_categoriesMappingOld[(*it).first];
        
        // for each format selected
        for ( MapInfoList::iterator mIt = ((*it).second).begin(); mIt != ((*it).second).end(); ++mIt )
        {
            // write appropriate results to grid
            resultFormat = m_formatsMapping[(*mIt).format]; 

            if ( resultFormat > -1 && resultFormat <= MODE )
            {            
                writeSummaryResults ((*mIt).gridMap, resultCat, resultFormat, i, j, timestepIndex);
            }
            else if ( resultFormat == PERCENTILE )
            {
                writePercentileResults ((*mIt).gridMap, (*mIt).percent, resultCat, i, j, timestepIndex);
            }
            else if ( resultFormat ==  DISTRIBUTION )
            {
                writeDistributionResults ((*mIt).gridMap, resultCat, i, j, timestepIndex);
            }
        }
    }
}
void MasterTouch::writeSummaryResults (GridMap *gridMap, short category, short format, int east, 
                                         int north, int timestepIndex)
{
   // prepare input for TS Summary call
   double summaryResults[MODE];

   double *summaryMode;
   long nMode;

   // call TS summary 
   if (TsLib_GetSummaryStatistics
       (timestepIndex,
        category,
        &summaryResults[MEAN],
        &summaryResults[SD],
        &summaryResults[SKEWNESS],
        &summaryResults[KURTOSIS], &summaryMode, &nMode, &summaryResults[MIN], &summaryResults[MAX]) != 1)
   {
      touchstoneError ("writeResults");
      return;
   }

   if (format != MODE)
   {
      // write result to 2d map
      // cerr << endl << "For Pos (" << east << "," << north << ") Summary Value = " << (summaryResults[format]) << endl;
      gridMap->setValue (east, north, (summaryResults[format]));
   }

   // free mode memory
   TsLib_FreeMode (summaryMode, nMode);
}
void MasterTouch::writePercentileResults (GridMap * gridMap, short percent, short category, int east, int north, int timestepIndex)
{
   double percentileValue;
      
   // call TS percentile
   if ( TsLib_GetPercentile
            (timestepIndex,
	     category,
	     percent,
	     &percentileValue) != 1 )
   {
      touchstoneError ("writeResults");
      return;
   }

   // cerr << endl << "For Pos (" << east << "," << north << ") Percentile Value (" << percent << ") = " << percentileValue << endl;
   gridMap->setValue (east, north, percentileValue);
}

void MasterTouch::writeDistributionResults (GridMap *gridMap, short category, int east, int north, int timestepIndex)
{
     
   //
   // 3D - NOT YET IMPLEMENTED
   //
   
   /*
   // call TS distribution
   double *distribution;
   long   *nDistribution;

   if ( TsLib_GetDistribution
            (timestepIndex,
	     category,
	     &distribution,
	     nDistribution) != 1 )
   {
      touchstoneError ("writeResults");
      return;
   }
   else
   {
      // save distribution results to something 3d

      // free distribution array memory
      TsLib_FreeDistribution (distribution, *nDistribution);
   }
   */
}

bool MasterTouch::retrieveGridMaps (void)
{
    for (CategoryMapInfoList::iterator cmilIterator = m_currentOutputs.begin (); cmilIterator != m_currentOutputs.end (); ++cmilIterator)
    {
        MapInfoList & mapInfoList = (*cmilIterator).second;
        
        for (MapInfoList::iterator milIterator = mapInfoList.begin (); milIterator != mapInfoList.end (); ++milIterator)
        {
            MapInfo & mapInfo = *milIterator;
            mapInfo.gridMap->retrieveData ();
        }
    }
    
    return true;
}
bool MasterTouch::restoreGridMaps (void)
{
    for (CategoryMapInfoList::iterator cmilIterator = m_currentOutputs.begin (); cmilIterator != m_currentOutputs.end (); ++cmilIterator)
    {
        MapInfoList & mapInfoList = (*cmilIterator).second;

        for (MapInfoList::iterator milIterator = mapInfoList.begin (); milIterator != mapInfoList.end (); ++milIterator)
        {
            MapInfo & mapInfo = *milIterator;
            mapInfo.gridMap->restoreData ();
        }
    }
    return true;
}

/** Mapping categories. */
int MasterTouch::SaveResultHeader ( TcfSchema::DetailHeadersType::modalHeaders_iterator& itor )
{
    for ( unsigned long int i = 0; i < iD.size(); ++i)
    {
        if ( itor->iD().kernel()     == iD[i].kernel() &&
             itor->iD().model()      == iD[i].model() &&
             itor->iD().iD()         == iD[i].iD() &&
             itor->iD().resultType() == iD[i].resultType() )
        {
            return i;
        }
    }
    return -1;
}
void MasterTouch::touchstoneError (const char *function)
{
    tsLib_errInfo * errInfo = TsLib_GetError();
 
    cerr << endl << "MasterTouch::" << function 
        << endl << errInfo->description
        << endl << errInfo->source << endl;
}

void MasterTouch::setCategoriesMapping ()
{
    // Total (Core) Porosity (volume %)
    iD.push_back(TcfSchema::ResultHeaderIdentificationType ( "model", 1, 5, 0));
    // Intergranular volume (volume %)
    iD.push_back(TcfSchema::ResultHeaderIdentificationType ( "model", 1, 0, 0));
    // Macro Porosity (volume %)
    iD.push_back(TcfSchema::ResultHeaderIdentificationType ( "modal", 0, 0, 4));
    // Micro Porosity (volume %)
    iD.push_back(TcfSchema::ResultHeaderIdentificationType ( "model", 1, 5, 1));
    // Permiability (md)
    iD.push_back(TcfSchema::ResultHeaderIdentificationType ( "model", 1, 6, 0));
    // Quartz (Volume %)
    iD.push_back(TcfSchema::ResultHeaderIdentificationType ( "model", 1, 3, 0));
    
    m_categoriesMapping [iCore_equiv_str]    = 0; //TSLIB_RC_CORE_PORO;
    m_categoriesMapping [iIntergranular_str] = 1; // TSLIB_RC_IGV;
    m_categoriesMapping [iMacro_str]         = 2; // TSLIB_RC_MACRO_PORO;
    m_categoriesMapping [iMicro_str]         = 3; //TSLIB_RC_MICRO_PORO;
    m_categoriesMapping [iAbsolute_str]      = 4; //TSLIB_RC_PERM;
    m_categoriesMapping [iCement_Quartz_str] = 5; //TSLIB_RC_CMT_QRTZ;
    m_categoriesMapping [iLog_str]           = 6; //TSLIB_RC_LOGPERM;

    m_categoriesMappingOld [iCore_equiv_str]    = TSLIB_RC_CORE_PORO;
    m_categoriesMappingOld [iIntergranular_str] = TSLIB_RC_IGV;
    m_categoriesMappingOld [iMacro_str]         = TSLIB_RC_MACRO_PORO;
    m_categoriesMappingOld [iMicro_str]         = TSLIB_RC_MICRO_PORO;
    m_categoriesMappingOld [iAbsolute_str]      = TSLIB_RC_PERM;
    m_categoriesMappingOld [iCement_Quartz_str] = TSLIB_RC_CMT_QRTZ;
    m_categoriesMappingOld [iLog_str]           = TSLIB_RC_LOGPERM;

}

/** Mappings format. */
void MasterTouch::setFormatsMapping (void)
{
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
    for (int i = 5; i < 99; i = i + 5)
    {
        m_percentPercentileMapping [i] = int (i/5);
    }
    m_percentPercentileMapping[ 99 ] = 20;   
    
}
