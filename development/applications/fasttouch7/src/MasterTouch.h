#ifndef FASTTOUCH7_MASTERTOUCH_H
#define FASTTOUCH7_MASTERTOUCH_H

#include <list>
#include <vector>
#include <map>
#include <string>
#include <tr1/array>

#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "petsc.h"

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

         // map of tcf filenames with corresponding output list
         typedef std::map < std::string, LayerCategoryMapInfoList > FileLayerCategoryMapInfoList;

         enum { SD = 0, MEAN, SKEWNESS, KURTOSIS, MIN, MAX, MODE, PERCENTILE, DISTRIBUTION };

      private:

         DataAccess::Interface::ProjectHandle & m_projectHandle;

         std::tr1::array<int,101> m_percentPercentileMapping;
         std::map < std::string, int > m_categoriesMapping;

         // display values
         std::map < std::string, int > m_formatsMapping;
         
         FileLayerCategoryMapInfoList  m_fileList;
         
        	std::vector<size_t> m_usedSnapshotsIndex;
        
         std::vector<double> m_usedSnapshotsAge;

         /** Collect cauldron output and call the new ts calculate*/
         bool calculate( const std::string & filename,
                         const DataAccess::Interface::Surface * surface,
                         const CategoryMapInfoList & currentOutputs );

         bool retrieveGridMaps(const CategoryMapInfoList & currentOutputs);
         bool restoreGridMaps(const CategoryMapInfoList & currentOutputs);
          
         /** save ts results to ts output directory */
         void writeResultsToGrids( int i, int j, const CategoryMapInfoList & currentOutputs, TouchstoneFiles & ReadTouchstone, size_t sn);

      public:
         MasterTouch( DataAccess::Interface::ProjectHandle & projectHandle);
         bool run();
     
         // add format request to category output for a certain layer
         bool addOutputFormat( const std::string & filename,
                               const DataAccess::Interface::Surface * surface,
                               const DataAccess::Interface::Formation * formation,
                               const std::string & category, const std::string & format, int percent);
   };

}


#endif

