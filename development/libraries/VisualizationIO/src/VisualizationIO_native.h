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

namespace CauldronIO
{
    struct DataStoreParams;

    /// \brief Map class implementation with native retrieve of data
    class MapNative : public Map
    {
    public:
        /// \brief Constructor defining if this map is cell centered, and its undefined value
        MapNative(bool cellCentered);
        
        /// \brief Override the retrieve method to load data from datastore
        virtual void retrieve();
        /// \brief Set all variables needed to retrieve the data; consider actually adding a reference to a datastore object that can do the logic
        void setDataStore(DataStoreParams* params);
        /// \brief Assign an associated depth surface by UUID
        void setDepthSurfaceUUID(const boost::uuids::uuid& uuid);
        /// \brief Returns true if a depthMap UUID has been assigned
        bool hasDepthMap() const;
        /// \brief Returns the depthmap UUID
        const boost::uuids::uuid& getDepthSurfaceUUID() const;

    private:
        DataStoreParams* m_params;
        bool m_hasDepthMap_uuid;
        boost::uuids::uuid m_uuid_depth;
    };

    /// \brief Volume class implementation with native retrieve of data
    class VolumeNative : public Volume
    {
    public:
        VolumeNative(bool cellCentered, SubsurfaceKind kind, boost::shared_ptr<const Property> property);

        /// \brief Override the retrieve method to load data from datastore
        virtual void retrieve();
        /// \brief Set all variables needed to retrieve the data; consider actually adding a reference to a datastore object that can do the logic
        void setDataStore(DataStoreParams* params, bool dataIJK);
        /// \brief Assign an associated depth surface by UUID
        void setDepthSurfaceUUID(const boost::uuids::uuid& uuid);
        /// \brief Returns true if a depthMap UUID has been assigned
        bool hasDepthMap() const;
        /// \brief Returns the depthmap UUID
        const boost::uuids::uuid& getDepthSurfaceUUID() const;

    private:
        bool m_hasDepthMap_uuid, m_dataIJK, m_dataKIJ;
        boost::uuids::uuid m_uuid_depth;
        DataStoreParams* m_paramsIJK;
        DataStoreParams* m_paramsKIJ;
    };

    /// \brief Volume class implementation with native retrieve of data
    class DiscontinuousVolumeNative : public DiscontinuousVolume
    {
    public:
        DiscontinuousVolumeNative();

        /// \brief Assign an associated depth surface by UUID
        void setDepthSurfaceUUID(const boost::uuids::uuid& uuid);
        /// \brief Returns true if a depthMap UUID has been assigned
        bool hasDepthMap() const;
        /// \brief Returns the depthmap UUID
        const boost::uuids::uuid& getDepthSurfaceUUID();

    private:
        bool m_hasDepthMap_uuid;
        boost::uuids::uuid m_uuid_depth;
    };
}
#endif
