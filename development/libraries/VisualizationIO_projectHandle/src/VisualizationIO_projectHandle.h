//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef __VisualizationIO_projectHandle_h__
#define __VisualizationIO_projectHandle_h__

#include "VisualizationAPI.h"
#include "GridMap.h"
#include "Formation.h"
#include "PropertyValue.h"

namespace CauldronIO
{
    /// Little structure to hold depth information about a formation
    struct FormationInfo
    {
        size_t kStart, kEnd;
        const DataAccess::Interface::Formation* formation;
        bool reverseDepth;
        
        // Cache some geometry values
        size_t numI, numJ;
        double deltaI, deltaJ, minI, minJ;
    };
    typedef vector<std::shared_ptr< FormationInfo> > FormationInfoList;
    
    /// \brief Implementation of Map class that can retrieve data from a ProjectHandle
    class MapProjectHandle : public SurfaceData
    {
    public:
        /// \brief Constructor defining if this map is cell centered, and its undefined value
        explicit MapProjectHandle(std::shared_ptr<const CauldronIO::Geometry2D>& geometry);
        ~MapProjectHandle();

        /// \brief Prefetch any data
        virtual void prefetch();
        /// \brief Override the retrieve method to load data from datastore
        virtual void retrieve();
        /// \brief Retrieve the data directly into the provided buffer
        virtual void retrieve(const ArrayView<float> &buffer) const override;
        /// \brief Release memory; does not destroy the object; it can be retrieved again
        virtual void release();
        /// \returns a list of HDFinfo holding the data; can be null
        virtual const std::vector < std::shared_ptr<HDFinfo> >& getHDFinfo();
        /// \brief Set all variables needed to retrieve the data
        void setDataStore(const DataAccess::Interface::PropertyValue* propVal);
		/// \brief Method to set data from an InputMap
		void setDataStore(const DataAccess::Interface::InputValue* inputVal);
        /// \returns true if all data needed is now ready (prefetch done)
        virtual bool signalNewHDFdata();

        /// Helper method
        static std::shared_ptr<CauldronIO::HDFinfo > getHDFinfoForPropVal(const DataAccess::Interface::PropertyValue* propVal);

    private:
        void retrieveFromHDF();
        const DataAccess::Interface::PropertyValue* m_propVal;
		const DataAccess::Interface::InputValue* m_inputVal;
        std::vector < std::shared_ptr<HDFinfo> > m_info;
    };

    /// \brief Implementation of Volume class that can retrieve data from a ProjectHandle
    class VolumeProjectHandle : public VolumeData
    {
    public:
		explicit VolumeProjectHandle(const std::shared_ptr<Geometry3D>& geometry);
        ~VolumeProjectHandle();

        /// \brief Prefetch any data: not implemented by this class
        virtual void prefetch();
        /// \brief Override the retrieve method to load data from datastore
        virtual void retrieve();
        /// \brief Retrieve the data directly into the provided buffer
        virtual void retrieve(const ArrayView<float> &buffer) const override;
        /// \brief Release memory; does not destroy the object; it can be retrieved again
        virtual void release();
        /// \returns a list of HDFinfo holding the data; can be null
        virtual const std::vector < std::shared_ptr<HDFinfo> >& getHDFinfo();
        /// \brief Set all variables needed to retrieve the data for multiple formations
        void setDataStore(std::shared_ptr<DataAccess::Interface::PropertyValueList> propValues,
            std::shared_ptr<CauldronIO::FormationInfoList> depthFormations);
        /// \brief Set all variables needed to retrieve the data for a single formation
        void setDataStore(const DataAccess::Interface::PropertyValue* propVal, std::shared_ptr<CauldronIO::FormationInfo> depthFormation);
        /// \brief Method to add HDF data to this class
        /// \returns true if all data needed is now ready (prefetch done)
        virtual bool signalNewHDFdata();

        /// \brief Helper method
        static std::shared_ptr<FormationInfo> findDepthInfo(std::shared_ptr<CauldronIO::FormationInfoList> depthFormations, const DataAccess::Interface::Formation* formation);

    private:

        void retrieveMultipleFromHDF();
        void retrieveSingleFromHDF();
        std::shared_ptr<DataAccess::Interface::PropertyValueList> m_propValues;
        std::shared_ptr<CauldronIO::FormationInfoList> m_depthFormations;
        const DataAccess::Interface::PropertyValue* m_propVal;
        std::shared_ptr<CauldronIO::FormationInfo> m_depthInfo;
        std::vector < std::shared_ptr<HDFinfo> > m_info;
    };
}
#endif
