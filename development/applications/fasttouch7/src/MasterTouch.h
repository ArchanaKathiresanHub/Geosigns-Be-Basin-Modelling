//                                                                      
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file MasterTouch.h
/// @brief This file keeps API declaration for adding ResQ outputs maps, running the touchstone wrapper and saving the results to maps. 
/// Note this class has linux functions calls and will not compile in windows

#ifndef FASTTOUCH7_MASTERTOUCH_H
#define FASTTOUCH7_MASTERTOUCH_H

#include <list>
#include <vector>
#include <map>
#include <string>
#include <tr1/array>
#include <iostream>

#include "Formation.h"
#include "Surface.h"
#include "petsc.h"
#include "BurialHistory.h"
#include "WriteBurial.h"

#include <sys/types.h>

static const int MAX_RUNS = 3;

namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
      class GridMap;
      class Grid;
   }
}

class TouchstoneFiles;

namespace fasttouch
{
   class MasterTouch
   {
   public:

      /// @brief Constructor
      MasterTouch(DataAccess::Interface::ProjectHandle & projectHandle);

      /// @brief Add a result map. Note for BPA2: an additional parameter (const std::string& runName ) is added to this method. 
      /// This value will be read from an additional column of the [TouchstoneIoTbl] (see the resq phase 2 requirement document)
      /// This is needed to support multiple facies maps in the same layer, similary to what it is done in BPA1, where several TCFs can be used in the same layer
      bool addOutputFormat(const std::string & filename,
         const DataAccess::Interface::Surface * surface,
         const DataAccess::Interface::Formation * formation,
         const std::string & category, const std::string & format, const int percent, const DataAccess::Interface::GridMap * faciesGrid, const int faciesNumber, const std::string & runName);

      /// @brief Write the burial histories, run the touchstoneWrapper and save the results to grid. this is done one TCF by one
      bool run();

   private:

      /// Class user defined types

      /// @brief type defining an output result, as read from the [TouchstoneIoTbl]
      struct MapInfo
      {
         std::vector<DataAccess::Interface::GridMap *> gridMap; /// vector containing the gridmaps at each user defined UseInResQ major snapshots
         std::string category;                                  /// category: MACRO_PORO, IGV, CMT_QRTZ, CORE_PORO, MICRO_PORO, PERM, LOGPERM
         std::string format;                                    /// format: SD, MEAN, GEOMEAN, SKEWNESS, KURTOSIS, MIN, MAX, MODE, PERCENTILE, DISTRIBUTION
         int percent;                                           /// percetiles: 1, 5, 10, ...,95, 99     
      };

      /// @brief define a type containing the information about one layer
      struct LayerInfo
      {
         LayerInfo() {}

         LayerInfo(const DataAccess::Interface::Surface   * surf,
            const DataAccess::Interface::Formation * form)
            : surface(surf), formation(form){}

         bool operator == (const LayerInfo & rhs) const
         {
            return surface == rhs.surface && formation == rhs.formation;
         }

         bool operator < (const LayerInfo & rhs) const
         {
            return surface == rhs.surface ?
               formation->getName() < rhs.formation->getName() : surface->getName() < rhs.surface->getName();
         }

         const DataAccess::Interface::Surface   * surface;
         const DataAccess::Interface::Formation * formation;
      };

      /// @brief define a type containing the information about one row of the [TouchstoneIoTbl] where the
      /// facies grid is specified with its facies number, the output layer and the MapInfo where to save the results
      struct FaciesGridMap
      {
         const DataAccess::Interface::GridMap * faciesGrid; /// the facies grid
         int faciesNumber;                                  /// the facies number defining the zone associated with its TCF file
         LayerInfo layer;                                   /// the layer 
         MapInfo * outputMap;                               /// the output map where to save the results
      };

      /// @brief define the categories
      enum { MACRO_PORO = 0, IGV, CMT_QRTZ, CORE_PORO, MICRO_PORO, PERM, LOGPERM, numberOfTouchstoneCategories };

      /// @brief define the formats
      enum { SD = 0, MEAN, GEOMEAN, SKEWNESS, KURTOSIS, MIN, MAX, MODE, PERCENTILE, DISTRIBUTION };

      /// Class methods

      /// @brief reads the results produced by the touchstoneWrapper and calls writeResultsToGrids
      /// @param filename the TCF file name
      /// @param burhistFile the name of the burial history file
      /// @param validLayerLocations for each layer the location 0 = burial history not saved, 1 = burial history saved
      bool calculate(const std::string & filename, const char * burhistFile, const std::map<LayerInfo, std::vector<int>> & validLayerLocations);

      /// @brief write the burial histories for each layer used by the TCF
      /// @param filename the TCF file name
      /// @param burhistFile the name of the burial history file
      /// @param validLayerLocations for each layer the location 0 = burial history not saved, 1 = burial history saved
      /// @param numActive the total number of active positions (must be known in advance to log the percentage of completion accurately)
      void writeBurialHistory(const string & filename, const char * burhistFile, const std::map<LayerInfo, std::vector<int>> & validLayerLocations, const int numActive);

      /// @brief executes the wrapper for a specific TCF file
      /// @param burhistFile the name of the burial history file
      /// @param filename the TCF file name
      /// @param resultFile the file where the touchstone results will be written
      bool executeWrapper(const char * burHistFile, const string & filename, const char * resultFile);

      /// @brief write the results of each i j position to the appropriate MapInfo    
      /// @param sn the snapshot number
      /// @param i the i position in the grid
      /// @param j the j position in the grid
      /// @param currentOutput the map info where the results will be written
      /// @param startingIndex the index where to start reading inside stripeOutput   
      /// @param stripeOutput the vector storing the TCF results
      void writeResultsToGrids(size_t sn, int i, int j, MapInfo * currentOutput, const size_t startingIndex, const std::vector<double>& stripeOutput);

      /// @brief checks if one instance of the wrapper is in the zombie state by reading the /proc/%d/stat file. Works only in linux
      bool checkZombie(pid_t pid);

      /// @brief prints logging messages
      void message(const std::string & msg, int level = 0) { if ( level >= m_verboseLevel ) (level > 0 ? std::cout : std::cerr) << "Basin_" << msg << std::endl; }

      /// Class members

      DataAccess::Interface::ProjectHandle & m_projectHandle;

      static const int numberOfStatisticalOutputs = 30;

      std::array<int, 101> m_percentPercentileMapping;

      std::map < std::string, int > m_categoriesMapping;

      std::vector<std::string> m_categories;

      std::map < std::string, int > m_formatsMapping;

      /// @brief define a type for storing the name of the output and its associated result map 
      /// first key of the map is an identifier for the output map. 
      /// In BPA1 the identifier is filename + category + format + percent. We need also the layer information in case the same TCF and output property is present  in several layers
      /// In BPA2 the identifier is is category + format + percent + runName (to support multifacies, where a TCF combination can be used more than once in a layer). We must have an unique
      /// run Name for each touchstone simulation.
      std::map < std::string, std::map<LayerInfo, MapInfo>>  m_fileMaps;

      /// @brief define a type for storing for each TCF file its faciesGridMap
      /// first key is the TCF file name, the second key is a vector of FaciesGridMap
      std::map < std::string, std::vector<FaciesGridMap> >  m_fileFacies;

      std::vector<size_t> m_usedSnapshotsIndex;

      std::vector<double> m_usedSnapshotsAge;

      int m_firstI;

      int m_firstJ;

      int m_lastI;

      int m_lastJ;

      int m_gridSize;

      PetscInt m_verboseLevel;
   };

}

#endif

