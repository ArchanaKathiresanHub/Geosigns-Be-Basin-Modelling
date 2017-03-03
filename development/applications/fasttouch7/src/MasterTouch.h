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

#ifndef FASTTOUCH7_MASTERTOUCH_H
#define FASTTOUCH7_MASTERTOUCH_H

#include <list>
#include <vector>
#include <map>
#include <string>
#include <tr1/array>
#include <iostream>

#include "Interface/Formation.h"
#include "Interface/Surface.h"
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

      /// @brief Add a result map. 
      /// Note for BPA2: an additional parameter (const std::string& runName ) needs to be added to this method. This value will be read from an additional column of the [TouchstoneIoTbl] (see the resq phase 2 requirment document)
      /// This is needed to support multiple facies maps in the same layer, similary to what it is done in BPA, where several TCFs can be used in the same layer
      bool addOutputFormat(const std::string & filename,
         const DataAccess::Interface::Surface * surface,
         const DataAccess::Interface::Formation * formation,
         const std::string & category, const std::string & format, const int percent, const DataAccess::Interface::GridMap * faciesGrid, const int faciesNumber);

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

      /// @brief define a type containing the information about one facies grid, the facies number and where to output the property
      struct FaciesGridMap
      {
         const DataAccess::Interface::GridMap * faciesGrid;
         int faciesNumber;
         LayerInfo layer;
         MapInfo * outputMap;
      };

      /// @brief define the categories
      enum { MACRO_PORO = 0, IGV, CMT_QRTZ, CORE_PORO, MICRO_PORO, PERM, LOGPERM, numberOfTouchstoneCategories };

      /// @brief define the formats
      enum { SD = 0, MEAN, GEOMEAN, SKEWNESS, KURTOSIS, MIN, MAX, MODE, PERCENTILE, DISTRIBUTION };

      /// Class methods

      /// @brief write the burial histories for each layer used by the TCF
      void writeBurialHistory(const string & filename, const char * burhistFile, const std::map<LayerInfo, std::vector<int>> & validLayerLocations, const int numActive);

      /// @brief reads the results produced by the touchstoneWrapper and calls writeResultsToGrids
      bool calculate(const std::string & filename, const char * burhistFile, const std::map<LayerInfo, std::vector<int>> & validLayerLocations);

      /// @brief executes the wrapper for a specific TCF file
      bool executeWrapper(const char * burHistFile, const string & filename, const char * resultFile);

      /// @brief write the results of each MapInfo to grid     
      void writeResultsToGrids(size_t sn, int i, int j, MapInfo * currentOutput, const size_t startingIndex, const std::vector<double>& stripeOutput);

      /// @brief checks if one instance of the wrapper is zombie state
      bool checkZombie(pid_t pid);

      /// @brief prints logging messages
      void message(const std::string & msg, int level = 0) { if ( level >= m_verboseLevel ) (level > 0 ? std::cout : std::cerr) << "MeSsAgE " << msg << std::endl; }

      /// Class members

      DataAccess::Interface::ProjectHandle & m_projectHandle;

      static const int numberOfStatisticalOutputs = 30;

      std::array<int, 101> m_percentPercentileMapping;

      std::map < std::string, int > m_categoriesMapping;

      std::vector<std::string> m_categories;

      std::map < std::string, int > m_formatsMapping;

      /// @brief define a type for storing the name of the output and its associated result map 
      std::map < std::string, MapInfo >  m_fileMaps;

      /// @brief define a type for storing for each TCF file its faciesGridMap
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

