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
   /** This class controls the functional requests to the ResQ library
    *  The necessary steps using the library are as follows
    *  - Load Tcf files associated with user-selected layer
    *  - Calculate all results for each grid point on the layer, based on
    *   the burial history and the Tcf file
    *  - Save user-requested results to maps
    */
   class MasterTouch
   {
   public:

      /** Struct containing Map information. */
      struct MapInfo
      {
         std::string format;
         std::vector<DataAccess::Interface::GridMap *> gridMap;
         int percent;
      };

      /** Struct containing information of different layers. */
      struct LayerInfo
      {
         LayerInfo() {}
            
         LayerInfo( const DataAccess::Interface::Surface   * surf,
                    const DataAccess::Interface::Formation * form )
            : surface( surf ), formation( form ) {}

         bool operator == ( const LayerInfo & rhs ) const
         {
            return surface == rhs.surface && formation == rhs.formation;
         }
            
         bool operator < ( const LayerInfo & rhs ) const
         {
            return surface == rhs.surface ?
               formation->getName() < rhs.formation->getName() : surface->getName() < rhs.surface->getName();
         }

         const DataAccess::Interface::Surface   * surface;
         const DataAccess::Interface::Formation * formation;
      };
         
      typedef std::list < MapInfo > MapInfoList;
         
      // output category and corresponding list of formats
      typedef std::map < std::string, MapInfoList > CategoryMapInfoList;
         
      // list of layers with corresponding categories for output
      typedef std::map < LayerInfo, CategoryMapInfoList > LayerCategoryMapInfoList;
         
      // map storing for each layer and category a counter, to push back only necessary maps
      typedef std::map < LayerInfo, std::map <std::string, int> > LayerCategoryResultCounter;

      enum { SD = 0, MEAN, GEOMEAN, SKEWNESS, KURTOSIS, MIN, MAX, MODE, PERCENTILE, DISTRIBUTION };
      
      enum { MACRO_PORO = 0, IGV, CMT_QRTZ, CORE_PORO, MICRO_PORO, PERM, LOGPERM};      
      
      void message( const std::string & msg, int level = 0 ) {  if ( level >= m_verboseLevel ) (level > 0 ? std::cout : std::cerr) << "MeSsAgE " << msg << std::endl; }      

      //** Struct containing facies map and facies indexes *//
      struct faciesGridMap
      {
         const DataAccess::Interface::GridMap * GridMap;
         int faciesNumber;
      };

      typedef std::map < LayerInfo, faciesGridMap > LayerFaciesGridMap;

      typedef std::map < std::string, LayerFaciesGridMap >  FileLayerFaciesGridMap;

   private:

      DataAccess::Interface::ProjectHandle & m_projectHandle;

      std::tr1::array<int,101> m_percentPercentileMapping;
      std::map < std::string, int > m_categoriesMapping;
      std::vector<std::string> m_categories;

      // display values
      std::map < std::string, int > m_formatsMapping;
         
      LayerCategoryMapInfoList  m_layerList;
         
      std::vector<size_t> m_usedSnapshotsIndex;
        
      std::vector<double> m_usedSnapshotsAge;
         
      PetscInt m_verboseLevel;         
         
      LayerCategoryResultCounter m_layerCategoryResultCounter;
         
      // Multi facies data mebers
         
      FileLayerFaciesGridMap m_fileLayerFaciesGridMap;

      /** Collect cauldron output and call the new ts calculate*/                         
      bool calculate( const std::string & filename, const char * burhistFile );

      bool retrieveGridMaps(const CategoryMapInfoList & currentOutputs);
      bool restoreGridMaps(const CategoryMapInfoList & currentOutputs);
      bool executeWrapper( const char * burHistFile, const string & filename, const char * resultFile );
          
      /** save ts results to ts output directory */

      void writeBurialHistory(const DataAccess::Interface::Surface * surface, WriteBurial & writeBurial, const faciesGridMap * faciesGridMap);
      
      int countActive( const DataAccess::Interface::Surface * surface, const faciesGridMap * faciesGridMap);
         
      void writeResultsToGrids( int i, int j, const CategoryMapInfoList & currentOutputs, TouchstoneFiles & ReadTouchstone, size_t sn);
         
   public:
      MasterTouch( DataAccess::Interface::ProjectHandle & projectHandle);
      bool run(); 
     
      // add format request to category output for a certain layer
      bool addOutputFormat( const std::string & filename,
                            const DataAccess::Interface::Surface * surface,
                            const DataAccess::Interface::Formation * formation,
                            const std::string & category, const std::string & format, int percent, const DataAccess::Interface::GridMap * faciesGrid, int index);

   };

}


#endif

