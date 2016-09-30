//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef __VisualizationUtils_h__
#define __VisualizationUtils_h__

#include "VisualizationAPI.h"
#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>

namespace CauldronIO
{
    /// \brief Class with utility functions
    class VisualizationUtils
    {
    public:
        /// \brief Load all data of given snapshot into memory using multiple threads, for VisualizationIO_projectHandle objects.
        //// since snapshot().retrieve does not work for reading HDF data for these objects, this method takes care of that
        /// \param[in] snapShot the snapshot to load
        /// \param[in] numThreads the number of threads to use
        static void retrieveAllData(const std::shared_ptr<SnapShot>& snapShot, size_t numThreads = 1);
        /// \brief Cell-centers all properties except depth for the maps in the current snapshot
        /// \param[in] snapShot the snapshot
        /// \param[in] project the project, to add the new geometries to the project
        static void cellCenterAllMaps(const std::shared_ptr<SnapShot>& snapShot, std::shared_ptr<Project>& project);
        /// \brief Cell-centers all properties except depth for given volume
        /// \param[in] snapShot the snapshot
        /// \param[in] project the project, to add the new geometries to the project
        static void cellCenterVolume(const std::shared_ptr<Volume>& volume, std::shared_ptr<Project>& project);
        /// \brief Cell-centers all properties except depth for set of formation volumes
        /// \param[in] snapShot the snapshot
        /// \param[in] project the project, to add the new geometries to the project
        static void cellCenterFormationVolumes(const std::shared_ptr<SnapShot>& snapShot, std::shared_ptr<Project> project);

    private:
        typedef float(*filterFunc)(float*, size_t);

        static std::shared_ptr<VolumeData> mergeVolumeDataList(PropertyVolumeDataList& propVolume, std::shared_ptr<Project> project);
        static std::shared_ptr<CauldronIO::SurfaceData> cellCenterMap(const std::shared_ptr<CauldronIO::SurfaceData>& map, filterFunc filter, filterFunc filterUndef);
        static std::shared_ptr<VolumeData> cellCenterVolumeData(const std::shared_ptr<VolumeData>& volData, filterFunc filter, filterFunc filterUndef);
        static void prefetchHDFdata(std::vector< VisualizationIOData* > allReadData, boost::lockfree::queue<int>* queue);
        static void loadHDFdata(std::vector< std::shared_ptr<HDFinfo> > hdfInfoList, boost::lockfree::queue<int>* queue);
        static void retrieveDataQueue(std::vector < VisualizationIOData* >* allData, boost::lockfree::queue<int>* queue, boost::atomic<bool>* done);
        static float getUndefAverage(float* v, size_t size);
        static float getAverage(float* v, size_t size);
        static float getUndefMedian(float* v, size_t size);
        static float getMedian(float* v, size_t size);
        static std::shared_ptr<VolumeData> mergeVolumes(std::shared_ptr<const Property>& propertyIO, std::vector<std::shared_ptr<VolumeData> > volDataList, std::shared_ptr<Project> project);
        static void computeVolumeData(std::shared_ptr<VolumeData> volData, std::shared_ptr<Geometry3D>& geometry, float* newData, size_t outputIndex,
            filterFunc filter, filterFunc filterUndef);
        static void getFilterFunc(const std::shared_ptr<const Property>& propertyIO, filterFunc& filter, filterFunc& filterUndef);
    };
}
#endif
