//                                                                      
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include <string>
#include <numeric>
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

double MinimumAll(double myValue);

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

static const char * tempBurial = "/tmp/BurialhistXXXXXX";
static const char * tempResults = "/tmp/ResultsXXXXXX";
static const char * tempStatus = "/tmp/StatusXXXXXX";

bool MasterTouch::checkZombie(pid_t pid)
{
#ifdef _WIN32
   return false;
#else
   char pbuf[32];

   snprintf(pbuf, sizeof(pbuf), "/proc/%d/stat", (int)pid);
   FILE* fpstat = fopen(pbuf, "r");

   // if "/proc/%d/stat" is not found we can not check the status of the zombie  	
   if ( !fpstat ) return false;

   int  rpid = 0;
   char rcmd[32];
   char rstatc = 0;

   fscanf(fpstat, "%d %30s %c", &rpid, rcmd, &rstatc);
   fclose(fpstat);
   return rstatc == 'Z' ? true : false;
#endif
}

bool MasterTouch::executeWrapper(const char * burHistFile, const string & filename, const char * resultFile) {

   char status[PATH_MAX];
   strcpy(status, tempStatus);
   mkstemp(status);
   mkfifo(status, 0777);

   ostringstream rank;
   rank << GetRank();

   int pid = fork();
   int childStatus = 0;

   if ( pid == 0 )
   {
      const char * wrapperName = "touchstoneWrapper";

      errno = 0;
      ibs::Path pathToWrapper = ibs::Path::applicationFullPath();
      pathToWrapper << wrapperName;

      // create a temporary file in the current directory to store the standard output (e.g. the warning messages from Matlab)
      string wrapperOut("./wrapperStandardOutput_");
      wrapperOut += rank.str();
      //read and write permissions for the user, read permission for the group  
      mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
      // copy standard output (1) to file
      int fd = open(wrapperOut.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
      dup2(fd, 1);
      close(fd);

      if ( pathToWrapper.exists() )
      {
         execl(pathToWrapper.path().c_str(), wrapperName,
            burHistFile, filename.c_str(), resultFile, status, (rank.str()).c_str(),
            static_cast<const char *>(0));

         if ( errno != 0 )
         {
            std::ostringstream oss;
            oss << "error: Could not run TouchstoneWrapper '" << pathToWrapper.path()
               << "  Error code " << errno << ":  " << std::strerror(errno);
            message(oss.str());
         }
      }
      else
      {
         message((std::string("error: could not find TouchstoneWrapper at: ") + pathToWrapper.path()).c_str());
      }
      exit(0);
   }

   double fractionCompleted = -10.0;
   double temFraction = -10.0;
   double minAllFractions = 0.01;
   double currentMinimum = 0.0;
   bool childStarted = false;
   int byteRead;
   int childstate;
   utilities::TimeToComplete timeToComplete(5, 30, minAllFractions, 0.01);
   int fd = open(status, O_RDONLY | O_NONBLOCK);
   timespec sleepTime;
   timespec remainingTime;

   sleepTime.tv_sec = 0;
   sleepTime.tv_nsec = 5000000L;

   ReportProgress("Loading touchstone and creating the realizations, please wait ...");

   // here we check several behaviours that can happen in the loading phase. 
   // If one child is bad we re-start all childrens. 
   do
   {
      // read the fractionCompleted first
      byteRead = read(fd, &temFraction, sizeof(temFraction));
      if ( byteRead == sizeof(temFraction) ) fractionCompleted = temFraction;

      // get the waitpidReturnValue and the childstate
      bool waitpidReturnValue = waitpid(pid, &childstate, WNOHANG);

      // the message string stream
      ostringstream oss;

      // if the child did not exit and computation started it is a good one
      // in case of no active elements in the domain, no computations are performed and fractionCompleted == 1
      if ( (!waitpidReturnValue && fractionCompleted > 0.0) || (waitpidReturnValue && fractionCompleted == 1.0) )
      {
         childStarted = true;
      }

      // if the child did not exit and is zombie it is a bad child
      else if ( !waitpidReturnValue && fractionCompleted < 0.0 && checkZombie(pid) )
      {
         oss << "warning: touchstoneWrapper is zombie " << std::strerror(errno);
         message(oss.str());
         childStatus = -1;
         childStarted = true;
      }

      // if the child exited badly before starting the computations it is a bad child
      else if ( waitpidReturnValue && fractionCompleted < 0.0 && !WIFEXITED(childstate) )
      {
         oss << "warning: touchstoneWrapper terminated prematurely " << std::strerror(errno);
         message(oss.str());
         childStatus = -1;
         childStarted = true;
      }

      // if the child exited ok before starting the computations it is a bad child (an exception was thrown and caught in Touchstone.C)
      else if ( waitpidReturnValue && fractionCompleted < 0.0 && WIFEXITED(childstate) )
      {
         oss << "warning: touchstoneWrapper thrown an exeception " << std::strerror(errno);
         message(oss.str());
         childStatus = -1;
         childStarted = true;
      }

      // sleep only after the checks ( waitpidReturnValue and childstate are up to date in the checks above). 
      nanosleep(&sleepTime, &remainingTime);
   } while ( !childStarted );

   currentMinimum = MinimumAll(childStatus);

   // return false if currentMinimum is < 0.0 
   if ( currentMinimum < 0.0 ) return false;

   timeToComplete.start();

   // Progress is reported only by Rank 0, we need to keep Rank 0 in the loop even if its child exited. 
   // One way to do this is to exit the while loop only if the child did not exit AND we are done in all ranks (currentMinimum < 1.0)
   while ( !waitpid(pid, &childstate, WNOHANG) || currentMinimum < 1.0 )
   {
      // read the fractionCompleted first. Calling MinimumAll requires all MPI processes to sync all times. This is expensive so we call it every 0.005 sec
      byteRead = read(fd, &temFraction, sizeof(temFraction));
      if ( byteRead == sizeof(temFraction) ) fractionCompleted = temFraction;

      currentMinimum = MinimumAll(fractionCompleted);

      if ( currentMinimum > minAllFractions )
      {
         minAllFractions = currentMinimum;
         string reported = timeToComplete.report(minAllFractions);
         if ( !reported.empty() ) ReportProgress(reported);
      }

      // sleep only after the check
      nanosleep(&sleepTime, &remainingTime);
   }
   if ( close(fd) < 0 )
   {
      ostringstream oss;
      oss << "error: Could not close status file, error code " << std::strerror(errno);
      message(oss.str());
      childStatus = -1;
   }
   if ( unlink(status) < 0 )
   {
      ostringstream oss;
      oss << "error: Could not unlink status file, error code " << std::strerror(errno);
      message(oss.str());
      childStatus = -1;
   }

   // if one child cannot close or unlink the status file, re-start all childrens.
   currentMinimum = MinimumAll(childStatus);

   return currentMinimum < 0.0 ? false : true;

}
// PUBLIC METHODS
//
/// The job of the constructor is to open the ResQ library and initialise
/// the format and category string lists
MasterTouch::MasterTouch(ProjectHandle & projectHandle)
   : m_projectHandle(projectHandle)
   , m_percentPercentileMapping()
   , m_categoriesMapping()
   , m_categories()
   , m_formatsMapping()
   , m_usedSnapshotsIndex()
   , m_usedSnapshotsAge()
   , m_verboseLevel(0)
{
   // set format mapping
   m_formatsMapping["Summary Standard Deviation"] = SD;
   m_formatsMapping["Summary Mean"] = MEAN;
   m_formatsMapping["Geometric Mean"] = GEOMEAN;
   m_formatsMapping["Summary Skewness"] = SKEWNESS;
   m_formatsMapping["Summary Kurtosis"] = KURTOSIS;
   m_formatsMapping["Summary Minimum"] = MIN;
   m_formatsMapping["Summary Maximum"] = MAX;
   m_formatsMapping["Summary Mode"] = MODE;
   m_formatsMapping["Percentile"] = PERCENTILE;
   m_formatsMapping["Distribution"] = DISTRIBUTION;


   // To default the percent for Percentile to 60 %.
   for ( int i = 0; i < 100; ++i )
   {
      m_percentPercentileMapping[i] = 12;
   }
   m_percentPercentileMapping[1] = 0;

   // Set now the available percents for Percentile.
   for ( int i = 5; i < 99; i = i + 5 )
   {
      m_percentPercentileMapping[i] = int(i / 5);
   }
   m_percentPercentileMapping[99] = 20;

   // set categories mapping 
   m_categories.push_back("Porosity Macro");
   m_categories.push_back("Porosity Intergranular Volume");
   m_categories.push_back("Cement Quartz");
   m_categories.push_back("Porosity Core Equivalent");
   m_categories.push_back("Porosity Micro");
   m_categories.push_back("Permeability Absolute");
   m_categories.push_back("Permeability Log10");

   //default indexing
   m_categoriesMapping[m_categories[MACRO_PORO]] = MACRO_PORO;
   m_categoriesMapping[m_categories[IGV]] = IGV;
   m_categoriesMapping[m_categories[CMT_QRTZ]] = CMT_QRTZ;
   m_categoriesMapping[m_categories[CORE_PORO]] = CORE_PORO;
   m_categoriesMapping[m_categories[MICRO_PORO]] = MICRO_PORO;
   m_categoriesMapping[m_categories[PERM]] = PERM;
   m_categoriesMapping[m_categories[LOGPERM]] = LOGPERM;

   // Used snapshots
   Interface::SnapshotList * majorSnapshots = m_projectHandle.getSnapshots(Interface::MAJOR);
   Interface::SnapshotList::iterator it;

   for ( size_t majorSnapshotIndex = 0; majorSnapshotIndex < majorSnapshots->size(); ++majorSnapshotIndex )
   {
      if ( (*majorSnapshots)[majorSnapshotIndex]->getUseInResQ() )
      {
         m_usedSnapshotsIndex.push_back(majorSnapshotIndex);
         m_usedSnapshotsAge.push_back((*majorSnapshots)[majorSnapshotIndex]->getTime());
      }
   }

   // Age 0 is always included
   if ( m_usedSnapshotsAge.empty() || find(m_usedSnapshotsAge.begin(), m_usedSnapshotsAge.end(), 0.0) == m_usedSnapshotsAge.end() )
   {
      m_usedSnapshotsIndex.push_back(0);
      m_usedSnapshotsAge.push_back(0.0);
   }

   //verbosity level
   delete majorSnapshots;
   PetscBool inputVerbose;
   PetscInt verboseLevel;

   PetscOptionsGetInt(PETSC_NULL, "-verboselevel", &verboseLevel, &inputVerbose);
   if ( inputVerbose )
   {
      m_verboseLevel = verboseLevel;
   }
}

bool MasterTouch::run()
{
   // make sure data has been selected 
   if ( m_fileFacies.empty() )
   {
      return false;
   }

   // the grid size
   m_firstI = m_projectHandle.getActivityOutputGrid()->firstI();
   m_firstJ = m_projectHandle.getActivityOutputGrid()->firstJ();
   m_lastI = m_projectHandle.getActivityOutputGrid()->lastI();
   m_lastJ = m_projectHandle.getActivityOutputGrid()->lastJ();
   m_gridSize = (m_lastI - m_firstI + 1) * (m_lastJ - m_firstJ + 1);

   // for each TCF file
   bool failure = false;
   for ( auto it = m_fileFacies.begin(); it != m_fileFacies.end(); ++it )
   {
      //the TCF file name
      const string & filename = (it->first);

      //for this TCF get the layers where it is used and create a mask to indicates the valid locations (at least one TCF is used)
      std::map<LayerInfo, std::vector<int>> validLayerLocations;
      for ( auto it = m_fileFacies[filename].begin(); it != m_fileFacies[filename].end(); ++it )
      {
         if ( validLayerLocations.count(it->layer) == 0 ) // new layer
         {
            validLayerLocations[it->layer] = std::vector<int>(m_gridSize, 0);
         }
      }

      // count the number of active areas and store them in a vector for later saving of the burial history (0 = not saved, 1 = saved)
      int totalNumBurialHistories = 0;
      for ( auto it = m_fileFacies[filename].begin(); it != m_fileFacies[filename].end(); ++it )
      {
         // if the count is done because all positions are active, no need to count again
         if ( std::accumulate(validLayerLocations[it->layer].begin(), validLayerLocations[it->layer].end(), 0) == m_gridSize ) continue;

         auto faciesGrid = it->faciesGrid;
         if ( faciesGrid ) faciesGrid->retrieveData(false);

         // retrive also the burial history to count all active timesteps
         BurialHistory burialHistory(it->layer.surface, m_projectHandle);
         size_t indexGrid = 0;
         for ( int i = m_firstI; i <= m_lastI; ++i )
         {
            for ( int j = m_firstJ; j <= m_lastJ; ++j )
            {
               // if the facies grid map is not defined, the entire stripe will be used
               if ( validLayerLocations[it->layer][indexGrid] != 1 and (!faciesGrid or faciesGrid->getValue((unsigned int)i, (unsigned int)j) == it->faciesNumber) )
               {
                  validLayerLocations[it->layer][indexGrid] = 1;
                  // count the total number of burial histories
                  const std::vector<BurialHistoryTimeStep> & burHistTimesteps = burialHistory.returnAsArray(i, j, true);
                  totalNumBurialHistories += burHistTimesteps.size();
               }
               indexGrid += 1;
            }
         }

         if ( faciesGrid ) faciesGrid->restoreData(false, false);
      }

      // Start the touchstone calculations
      string progressString = "Starting TCF: ";
      progressString += filename;
      ReportProgress(progressString);

      //write burial histories for all layers and facies that uses that TCF file
      char burhistFile[PATH_MAX];
      strcpy(burhistFile, tempBurial);
      mkstemp(burhistFile);

      //write the burial history
      writeBurialHistory(filename, burhistFile, validLayerLocations, totalNumBurialHistories);

      // run touchstone wrapper      
      // check if failure needs to be simulated
      char * touchstoneWrapperFailure = getenv("touchstoneWrapperFailure");

      bool calculated = false;
      for ( int runs = 1; runs <= MAX_RUNS && !calculated; ++runs )
      {
         if ( touchstoneWrapperFailure && GetRank() == atol(touchstoneWrapperFailure) )
         {
            calculated = calculate("WrongTCF", burhistFile, validLayerLocations);
         }
         else
         {
            calculated = calculate(filename, burhistFile, validLayerLocations);
         }

         if ( !calculated )
         {
            std::ostringstream oss;
            oss << "warning: MasterTouch::calculate is restarted on MPI process " << GetRank() << " after " << runs << " runs";
            message(oss.str());
         }
      }

      while ( MinimumAll(10)<10 );

      if ( !calculated )
      {
         failure = true;
         break;
      }

      progressString = "Finished TCF: ";
      progressString += filename;
      ReportProgress(progressString);
   }

   return (!failure);

}

bool MasterTouch::addOutputFormat(const string & filename,
   const Surface * surface, const Formation * formation,
   const string & category, const string & format,
   const int percent, const GridMap * faciesGrid, const int faciesNumber)
{
   if ( filename.size() < 1 )
   {
      ostringstream oss;
      oss << "warning: MasterTouch::addOutputFormat, a tcf file has not been chosen ";
      message(oss.str());
      return false;
   }

   string propertyValueName;

   propertyValueName += "Resq: ";
   const bool isLegacy = m_projectHandle.getRunParameters()->getLegacy();
   if ( isLegacy )
   {
      propertyValueName += filename;
      propertyValueName += " ";
   }

   propertyValueName += category;
   propertyValueName += " ";
   propertyValueName += format;

   propertyValueName += " ";
   propertyValueName += to_string(percent);

   if ( !isLegacy )
   {
      // Here we  add the run name read from the project file as an additional parameter of the [TouchstoneIoTbl]
      // This should be passed to addOutputFormat and it is necessary to support multifacies in ANY setting, e.g. several multifacies scenarios in the same layer. 
      // propertyValueName += " ";
      // propertyValueName += runName; //scenNumber, should be read from the TouchstoneIoTbl
   }

   LayerInfo layer(surface, formation);

   //if a map for this format, category and percent has been already defined no need to create a new MapInfo
   if ( m_fileMaps[propertyValueName].count(layer) == 0 )
   {

      //Create map info
      MapInfo map;
      map.format = format;
      map.percent = percent;
      map.category = category;

      for ( size_t it = 0; it < m_usedSnapshotsAge.size(); ++it )
      {

         const Snapshot * majorSnapshot = m_projectHandle.findSnapshot(m_usedSnapshotsAge[it]);

         if ( !majorSnapshot )
         {
            ostringstream oss;
            oss << "warning: MasterTouch::addOutputFormat, Could not create PropertyValue: " << propertyValueName <<
               ", could not find snapshot " << m_usedSnapshotsAge[it];
            message(oss.str());
            return false;
         }

         PropertyValue * propertyValue = m_projectHandle.createMapPropertyValue(propertyValueName, majorSnapshot, 0, formation, surface);

         if ( !propertyValue )
         {
            ostringstream oss;
            oss << "warning: MasterTouch::addOutputFormat, Could not create PropertyValue named: " << propertyValueName;
            message(oss.str());
            return false;
         }

         GridMap * newGridMap = propertyValue->getGridMap();
         map.gridMap.push_back(newGridMap);
      }

      // for each scenario and property we have only one stack of maps
      m_fileMaps[propertyValueName][layer] = map;
   }

   //save where is used and where to write 
   FaciesGridMap faciesGridMap;  
   faciesGridMap.faciesGrid = faciesGrid;
   faciesGridMap.faciesNumber = faciesNumber;
   faciesGridMap.layer = layer;
   faciesGridMap.outputMap = &(m_fileMaps[propertyValueName][layer]);

   // for each filename store the faciesGridMap to be saved to output
   m_fileFacies[filename].push_back(faciesGridMap);

   return true;
}

void MasterTouch::writeBurialHistory(const string & filename, const char * burhistFile, const std::map<LayerInfo, std::vector<int>> & validLayerLocations, const int totalNumBurialHistories)
{

   WriteBurial writeBurial(burhistFile);
   writeBurial.writeIndexes(m_firstI, m_lastI, m_firstJ, m_lastJ, validLayerLocations.size(), totalNumBurialHistories);
   writeBurial.writeSnapshotsIndexes(m_usedSnapshotsIndex);

   for ( auto it = validLayerLocations.begin(); it != validLayerLocations.end(); ++it )
   {
      BurialHistory burialHistory(it->first.surface, m_projectHandle);
      size_t indexGrid = 0;
      for ( int i = m_firstI; i <= m_lastI; ++i )
      {
         for ( int j = m_firstJ; j <= m_lastJ; ++j )
         {
            int iD = (i - m_firstI)  * (m_lastJ - m_firstJ + 1) + j;

            if ( it->second[indexGrid] == 0 )
            {
               writeBurial.writeBurialHistory(nullptr, iD);
            }
            else
            {
               const std::vector<BurialHistoryTimeStep> & burHistTimesteps = burialHistory.returnAsArray(i, j, true);
               writeBurial.writeBurialHistory(&burHistTimesteps, iD);
            }
            indexGrid += 1;
         }
      }
   }
}

bool MasterTouch::calculate(const std::string & filename, const char * burhistFile, const std::map<LayerInfo, std::vector<int>> & validLayerLocations)
{
   try
   {
      char resultFile[PATH_MAX];
      strcpy(resultFile, tempResults);
      mkstemp(resultFile);

      //Execute touchstoneWrapper
      if ( !executeWrapper(burhistFile, filename, resultFile) )
      {
         throw std::runtime_error("Error in executeWrapper");
      }
      //remove burial history
      remove(burhistFile);

      //read the results
      TouchstoneFiles readTouchstone(resultFile);

      //read the categories as saved by touchstone library
      std::vector<int> vec;
      readTouchstone.readOrder(vec);
      for ( size_t ii = 0; ii < vec.size() - 1; ++ii ) m_categoriesMapping[m_categories[ii]] = vec[ii];

      //Read the outputs. Will store all outputs of this TCF here, allows combining the outputs in different facies maps
      std::map<LayerInfo, std::vector<double>> stripeOutput;
      std::map<LayerInfo, std::vector<int>> validTimeSteps;
      size_t numberOfOutputs = numberOfTouchstoneCategories * numberOfStatisticalOutputs;

      for ( auto it = validLayerLocations.begin(); it != validLayerLocations.end(); ++it )
      {
         //count the number of active position for this layer and allocate the required memory
         int layerNumActive = std::accumulate(it->second.begin(), it->second.end(), 0);
         stripeOutput[it->first] = std::vector<double>(layerNumActive* m_usedSnapshotsIndex.size() *numberOfOutputs, 99999.0);
         validTimeSteps[it->first] = std::vector<int>(m_gridSize, 0);
         
         size_t startingIndex = 0;
         size_t indexTimeSteps = 0;

         for ( int i = m_firstI; i <= m_lastI; ++i )
         {
            for ( int j = m_firstJ; j <= m_lastJ; ++j )
            {
               //num of timeSteps should always be read
               size_t numTimeSteps = 0;
               readTouchstone.readNumTimeSteps(&numTimeSteps);
               validTimeSteps[it->first][indexTimeSteps] = numTimeSteps;
               indexTimeSteps += 1;

               // read the output arrays only if numTimeSteps > 0
               if ( numTimeSteps > 0 )
               {
                  for ( size_t sn = 0; sn < m_usedSnapshotsIndex.size(); ++sn )
                  {
                     std::vector<double> needleOutput(numberOfOutputs, 99999.0);
                     readTouchstone.readArray(needleOutput);

                     //copy the results to the output vector
                     std::copy(needleOutput.begin(), needleOutput.end(), stripeOutput[it->first].begin() + startingIndex);
                     startingIndex += numberOfOutputs;
                  }
               }
            }
         }
      }

      // fill each result map belonging to this filename, read the results
      for ( auto it = m_fileFacies[filename].begin(); it != m_fileFacies[filename].end(); ++it )
      {
         // retrive the current map output data
         MapInfo * currentOutput = it->outputMap;
         for ( auto const&  m: currentOutput->gridMap ) m->retrieveData();

         // if a facies is not defined, all surface belongs to the TCF
         bool facieGridMapisDefined = false;
         if ( it->faciesGrid )
         {
            facieGridMapisDefined = true;
            it->faciesGrid->retrieveData(false);
         }

         size_t startingIndex = 0;
         size_t indexTimeSteps = 0;
         for ( int i = m_firstI; i <= m_lastI; ++i )
         {
            for ( int j = m_firstJ; j <= m_lastJ; ++j )
            {
               if ( validTimeSteps[it->layer][indexTimeSteps] > 0 ) // Valid results are present at this location
               {
                  for ( size_t sn = 0; sn < m_usedSnapshotsIndex.size(); ++sn )
                  {
                     // the results are part of the facies map defined in MapInfo. If no faciesGrid is defined it means that all TCF will be used in the stripe
                     if ( !facieGridMapisDefined or it->faciesGrid->getValue((unsigned int)i, (unsigned int)j) == it->faciesNumber )
                     {
                        writeResultsToGrids(sn, i, j, currentOutput, startingIndex, stripeOutput[it->layer]);
                     }
                     startingIndex += numberOfOutputs;
                  }
               }
               indexTimeSteps += 1;
            }
         }

         if ( facieGridMapisDefined ) it->faciesGrid->restoreData(false, false);
         // restore the current output map 
         for ( auto const& m : currentOutput->gridMap ) m->restoreData();
      }
   }
   catch ( const std::runtime_error & e )
   {
      ostringstream oss;
      oss << "error: MasterTouch::calculate() error on MPI process " << GetRank() << " : " << e.what();
      message(oss.str());
      return false;
   }
   catch ( std::exception & e )
   {
      ostringstream oss;
      oss << "error: MasterTouch::calculate() error on MPI process " << GetRank() << " : " << e.what();
      message(oss.str());
      return false;
   }
   catch ( ... )
   {
      ostringstream oss;
      oss << "error: MasterTouch::calculate() unknown  error on MPI process " << GetRank();
      message(oss.str());
      return false;
   }

   return true;
}

void MasterTouch::writeResultsToGrids(size_t sn, int i, int j, MapInfo * currentOutput, const size_t startingIndex, const std::vector<double>& stripeOutput)
{
   // for each format selected
   int resultCat = m_categoriesMapping[currentOutput->category];
   int resultFormat = m_formatsMapping[currentOutput->format];
   // Explanation to formula below:
   // if resultFormat = {SD = 0, 1, 2, 3, 4, 5, MODE = 7} then resultStat = resultFormat.
   // if resultFormat = { MODE } then resultStat = 8, ... 29 
   // if resultFormat = { DISTRIBUTION } then resultStat = 30;
   int resultStat = resultFormat
      + int(resultFormat > MODE) * (m_percentPercentileMapping[currentOutput->percent])
      + int(resultFormat > PERCENTILE) * (20 - m_percentPercentileMapping[currentOutput->percent]);
   currentOutput->gridMap[sn]->setValue(i, j, stripeOutput[startingIndex + resultCat *  numberOfStatisticalOutputs + resultStat]);
}