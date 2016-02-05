//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef __VisualizationIO_native_h__
#define __VisualizationIO_native_h__

#include "VisualizationAPI.h"
#include "Interface/GridMap.h"
#include "Interface/Formation.h"
#include "Interface/PropertyValue.h"

namespace CauldronIO
{
    /// Little structure to hold depth information about a formation
    struct FormationInfo
    {
        double depthStart, depthEnd;
        size_t kStart, kEnd;
        const DataAccess::Interface::Formation* formation;
        bool reverseDepth;
        const DataAccess::Interface::PropertyValue* propValue;
        
        // Cache some geometry values
        size_t numI, numJ;
        double deltaI, deltaJ, minI, minJ;

        static bool compareFormations(boost::shared_ptr<CauldronIO::FormationInfo> info1, boost::shared_ptr < CauldronIO::FormationInfo> info2);
    };
    typedef vector<boost::shared_ptr< FormationInfo> > FormationInfoList;
    
    /// \brief Implementation of Map class that can retrieve data from a ProjectHandle
    class MapProjectHandle : public SurfaceData
    {
    public:
        /// \brief Constructor defining if this map is cell centered, and its undefined value
        MapProjectHandle(boost::shared_ptr<const CauldronIO::Geometry2D>& geometry);
        
        /// \brief Override the retrieve method to load data from datastore
        virtual void retrieve();
        /// \brief Release memory; does not destroy the object; it can be retrieved again
        virtual void release();
        /// \brief Set all variables needed to retrieve the data
        void setDataStore(const DataAccess::Interface::PropertyValue* propVal);

    private:
        const DataAccess::Interface::PropertyValue* m_propVal;
    };

    /// \brief Implementation of Volume class that can retrieve data from a ProjectHandle
    class VolumeProjectHandle : public VolumeData
    {
    public:
        VolumeProjectHandle(const boost::shared_ptr<const Geometry3D>& geometry);

        /// \brief Override the retrieve method to load data from datastore
        virtual void retrieve();
        /// \brief Release memory; does not destroy the object; it can be retrieved again
        virtual void release();
        /// \brief Set all variables needed to retrieve the data for multiple formations
        void setDataStore(boost::shared_ptr<DataAccess::Interface::PropertyValueList> propValues,
            boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations);
        /// \brief Set all variables needed to retrieve the data
        void setDataStore(const DataAccess::Interface::PropertyValue* propVal, boost::shared_ptr<CauldronIO::FormationInfo> depthFormation);

        /// \brief Helper method
        static boost::shared_ptr<FormationInfo> findDepthInfo(boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations, const DataAccess::Interface::Formation* formation);

    private:
        void retrieveMultipleFormations();
        void retrieveSingleFormation();
        boost::shared_ptr<DataAccess::Interface::PropertyValueList> m_propValues;
        boost::shared_ptr<CauldronIO::FormationInfoList> m_depthFormations;
        const DataAccess::Interface::PropertyValue* m_propVal;
        boost::shared_ptr<CauldronIO::FormationInfo> m_depthInfo;
    };
}
#endif
