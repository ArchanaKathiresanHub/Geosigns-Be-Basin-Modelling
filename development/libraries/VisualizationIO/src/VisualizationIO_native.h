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
    /// \brief Map class implementation with native retrieve of data
    class MapNative : public Map
    {
    public:
        /// \brief Constructor defining if this map is cell centered, and its undefined value
        MapNative(bool cellCentered);
        
        /// \brief Override the retrieve method to load data from datastore
        virtual void retrieve();
        /// \brief Set all variables needed to retrieve the data; consider actually adding a reference to a datastore object that can do the logic
        void setDataStore(const std::string& filename, bool compressed, size_t offset, size_t size);
        /// \brief Assign an associated depth surface by UUID
        void setDepthSurfaceUUID(const boost::uuids::uuid& uuid);

    private:
        std::string m_filename;
        bool m_compressed;
        size_t m_offset, m_size;
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
        void setDataStore(const std::string& filename, bool compressed, size_t offset, size_t size);

    private:
        std::string m_filename;
        bool m_compressed;
        size_t m_offset, m_size;
    };
}
#endif
