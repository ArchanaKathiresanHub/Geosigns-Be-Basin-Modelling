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

        static bool CompareFormations(const FormationInfo* info1, const FormationInfo* info2);
    };
    typedef vector<FormationInfo*> FormationInfoList;
    
    /// \brief Implementation of Map class that can retrieve data from a ProjectHandle
    class MapProjectHandle : public Map
    {
    public:
        /// \brief Constructor defining if this map is cell centered, and its undefined value
        MapProjectHandle(bool cellCentered);
        
        /// \brief Override the retrieve method to load data from datastore
        virtual void Retrieve();
        /// \brief Set all variables needed to retrieve the data
        void SetDataStore(const DataAccess::Interface::PropertyValue* propVal);

    private:
        const DataAccess::Interface::PropertyValue* _propVal;
    };

    /// \brief Implementation of Volume class that can retrieve data from a ProjectHandle
    class VolumeProjectHandle : public Volume
    {
    public:
        VolumeProjectHandle(bool cellCentered, SubsurfaceKind kind, boost::shared_ptr<const Property> property);

        /// \brief Override the retrieve method to load data from datastore
        virtual void Retrieve();
        /// \brief Set all variables needed to retrieve the data for multiple formations
        void SetDataStore(boost::shared_ptr<DataAccess::Interface::PropertyValueList> propValues,
            boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations);
        /// \brief Set all variables needed to retrieve the data
        void SetDataStore(const DataAccess::Interface::PropertyValue* propVal, const CauldronIO::FormationInfo* depthFormation);

        /// \brief Helper method
        static FormationInfo* FindDepthInfo(boost::shared_ptr<CauldronIO::FormationInfoList> depthFormations, const DataAccess::Interface::Formation* formation);

    private:
        void RetrieveMultipleFormations();
        void RetrieveSingleFormation();
        boost::shared_ptr<DataAccess::Interface::PropertyValueList> _propValues;
        boost::shared_ptr<CauldronIO::FormationInfoList> _depthFormations;
        const DataAccess::Interface::PropertyValue* _propVal;
        const CauldronIO::FormationInfo* _depthInfo;
    };
}
#endif
