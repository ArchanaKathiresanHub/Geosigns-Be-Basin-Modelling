#ifndef __mastertouch__
#define __mastertouch__

#include "tslibI.h"
#include "TsLibLoader.h"

#include "TsLib.h"

#include "hdf5.h"

#include <list>
#include <vector>
#include <map>
#include <string>

#include <iostream>
#include <fstream>

using namespace std;

#include "Interface/Formation.h"
#include "Interface/Surface.h"

#include "BurialHistoryGeneric.h"

namespace DataAccess
{
    namespace Interface
    {
        class GridMap;
        class Grid;
    }
}

namespace fasttouch
{
    //class BurialHistory;
    class FastTouch;
    
    /** This class controls the functional requests to the ResQ library
     *  The necessary steps using the library are as follows
     *  - Load Tcf files associated with user-selected layer
     *  - Calculate all results for each grid point on the layer, based on
     *    the burial history and the Tcf file
     *  - Save user-requested results to maps
     */
    class MasterTouch
    {
        public:
            /** Struct containing Map information. */
            struct MapInfo
            {
                string format;
                DataAccess::Interface::GridMap * gridMap;
                short percent;
            };
            /** Class containing information of different layers. */
            class LayerInfo
            {
                public:
                    LayerInfo ()
                    {
                    }
                    
                    LayerInfo ( const DataAccess::Interface::Surface * surf,
                                const DataAccess::Interface::Formation * form )
                        : surface (surf), formation (form)
                    {
                    }

                    bool operator== (const LayerInfo & rhs) const
                    {
                        return surface == rhs.surface && formation == rhs.formation;
                    }
                    
                    bool operator< (const LayerInfo & rhs) const
                    {
                        return surface == rhs.surface ?
                            formation->getName () < rhs.formation->getName () : surface->getName () < rhs.surface->getName ();
                    }
                    const DataAccess::Interface::Surface * surface;
                    const DataAccess::Interface::Formation * formation;
            };
            
            typedef list < MapInfo > MapInfoList;
            
            // output category and corresponding list of formats
            typedef map < string, MapInfoList > CategoryMapInfoList;
            
            // list of layers with corresponding categories for output
            typedef map < LayerInfo, CategoryMapInfoList > LayerCategoryMapInfoList;

            // map of tcf filenames with corresponding output list
            typedef map < string, LayerCategoryMapInfoList > FileLayerCategoryMapInfoList;

            enum { SD = 0, MEAN, SKEWNESS, KURTOSIS, MIN, MAX, MODE, PERCENTILE, DISTRIBUTION };
        private:
            static const int numberOfTouchstoneProperties = 7;
            static const int numberOfStatisticalOutputs   = 29;
            double m_OutputProperties [ numberOfTouchstoneProperties * numberOfStatisticalOutputs ];
            double m_ActualOutputProperties [ numberOfTouchstoneProperties * numberOfStatisticalOutputs ];
            /** Result header types of models and modals to be saved are stored
             *  in this list. */
            std::vector < TcfSchema::ResultHeaderIdentificationType > iD;
            /** Determine if the given result header is to be saved or not. */
            int SaveResultHeader (TcfSchema::DetailHeadersType::modalHeaders_iterator& itor);

            map <int, int> m_categoriesMappingOrder;
            int m_percentPercentileMapping [101];
            map < string, int > m_categoriesMapping;
            map < string, int > m_categoriesMappingOld;
            FastTouch * m_fastTouch;

            // display values
            map < string, int > m_formatsMapping;
            
            LayerCategoryMapInfoList *m_layerList;
            FileLayerCategoryMapInfoList m_fileList;

            CategoryMapInfoList m_currentOutputs;

            // make sure tcf file is valid file type
            bool validTcfFile (char *filename);

            /** Touchstone Library Interface object.*/ 
            Geocosm::TsLib::TsLibInterface* tslib;
            
            /** Switch to determine whether the touchstone library calculation
             * (tslib->Calculate(...)) is a direct analog run or monte carlo
             * run. Output of tcfinfo->IsDirectAnalogRun() is stored in this
             * variable. */
            bool m_directAnalogRun;
            
            /** Number of realizations for monte carlo run. */
            int m_nrOfRealizations;

            /** Touchstone configuration file object.*/
            Geocosm::TsLib::TcfInfoInterface* tcfInfo;

            /** Touchstone calculation context. */
            Geocosm::TsLib::CalcContextInterface* tslibCalcContext;

            /** Touchstone BurialHistory info.*/
            Geocosm::TsLib::TsLibBurialHistory tslibBurialHistoryInfo;
            /** Collect cauldron output and call the new ts calculate*/
            bool calculate ( const char *filename,
                             const DataAccess::Interface::Surface * surface,
                             const DataAccess::Interface::Formation * formation );
            /** collect cauldron output and call the old ts calculate */
            bool calculateOld ( const char *filename,
                             const DataAccess::Interface::Surface * surface,
                             const DataAccess::Interface::Formation * formation );

            bool retrieveGridMaps (void);
            bool restoreGridMaps (void);
            
            void touchstoneError (const char *function);

            // save ts results to ts output directory
            void writeResultsToGrids (int east, int north, int timestepIndex);
            void writeResultsToGridsOld (int east, int north, int timestepIndex);
            void writeSummaryResults (DataAccess::Interface::GridMap * gridMap, short category, short format, int east, int north, int timestepIndex);
            void writePercentileResults (DataAccess::Interface::GridMap * gridMap, short percent, short category, int east, int north,
                    int timestepIndex);
            void writeDistributionResults (DataAccess::Interface::GridMap * gridMap, short category, int east, int north, int timestepIndex);
            
            void setFormatsMapping (void);
            void setCategoriesMapping (void);

            // memmory deallocation functions
            void cleanUpRun (char **filenames, int size);

        public:
            MasterTouch (FastTouch * fastTouch);
            bool run ();
      
            // add format request to category output for a certain layer
            bool addOutputFormat (const string & filename,
                    const DataAccess::Interface::Surface * surface,
                    const DataAccess::Interface::Formation * formation,
                    const string & category, const string & format, short percent);

            ~MasterTouch (void);
    };

    // h5 compound type for Result grid coordinates
    struct GridCoordInfo
    {
        GridCoordInfo ( double minE, double maxE, double minN, double maxN,
                        float delI, float delJ, int minx, int maxx, int miny, int maxy )
            : minEast (minE), maxEast (maxE), minNorth (minN),
            maxNorth (maxN), deltaI (delI), deltaJ (delJ), minX (minx), maxX (maxx), minY (miny), maxY (maxy)
        {
        }

        GridCoordInfo ()
        {
        }

        ~GridCoordInfo ()
        {
        }

        // GridCoordInfo& operator[] (int index)  

        double minEast;
        double maxEast;
        double minNorth;
        double maxNorth;
        float deltaI;
        float deltaJ;
        int minX;
        int maxX;
        int minY;
        int maxY;
    };

    class H5_ResultInfo
    {
        public:
            // ctor / dtor
            H5_ResultInfo (void)
            {
                GridCoordInfo gridCoordInfo;
                
                hGridId = H5Tcreate (H5T_COMPOUND, sizeof (GridCoordInfo));
                
                H5Tinsert (hGridId, "MinEast", (size_t) ((size_t) & gridCoordInfo.minEast - (size_t) & gridCoordInfo),
                        H5T_NATIVE_DOUBLE);
                H5Tinsert (hGridId, "MaxEast", (size_t) ((size_t) & gridCoordInfo.maxEast - (size_t) & gridCoordInfo),
                        H5T_NATIVE_DOUBLE);
                H5Tinsert (hGridId, "MinNorth", (size_t) ((size_t) & gridCoordInfo.minNorth - (size_t) & gridCoordInfo),
                        H5T_NATIVE_DOUBLE);
                H5Tinsert (hGridId, "MaxNorth", (size_t) ((size_t) & gridCoordInfo.maxNorth - (size_t) & gridCoordInfo),
                        H5T_NATIVE_DOUBLE);
                H5Tinsert (hGridId, "DeltaI", (size_t) ((size_t) & gridCoordInfo.deltaI - (size_t) & gridCoordInfo),
                        H5T_NATIVE_FLOAT);
                H5Tinsert (hGridId, "DeltaJ", (size_t) ((size_t) & gridCoordInfo.deltaJ - (size_t) & gridCoordInfo),
                        H5T_NATIVE_FLOAT);
                H5Tinsert (hGridId, "MinX", (size_t) ((size_t) & gridCoordInfo.minX - (size_t) & gridCoordInfo),
                        H5T_NATIVE_INT);
                H5Tinsert (hGridId, "MaxX", (size_t) ((size_t) & gridCoordInfo.maxX - (size_t) & gridCoordInfo),
                        H5T_NATIVE_INT);
                H5Tinsert (hGridId, "MinY", (size_t) ((size_t) & gridCoordInfo.minY - (size_t) & gridCoordInfo),
                        H5T_NATIVE_INT);
                H5Tinsert (hGridId, "MaxY", (size_t) ((size_t) & gridCoordInfo.maxY - (size_t) & gridCoordInfo),
                        H5T_NATIVE_INT);
            }
            
            ~H5_ResultInfo (void)
            {
                H5Tclose (hGridId);
            }

            // public methods
            hid_t gridId (void) const
            {
                return hGridId;
            }
        private:
            hid_t hGridId;
    };
}

#endif

