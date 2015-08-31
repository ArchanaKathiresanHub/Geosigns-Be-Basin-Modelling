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
        virtual void Retrieve();
        /// \brief Set all variables needed to retrieve the data; consider actually adding a reference to a datastore object that can do the logic
        void SetDataStore(const std::string& filename, bool compressed, size_t offset, size_t size);

    private:
        std::string _filename;
        bool _compressed;
        size_t _offset, _size;
    };

    /// \brief Volume class implementation with native retrieve of data
    class VolumeNative : public Volume
    {
    public:
        VolumeNative(bool cellCentered, SubsurfaceKind kind, boost::shared_ptr<const Property> property);

        /// \brief Override the retrieve method to load data from datastore
        virtual void Retrieve();
        /// \brief Set all variables needed to retrieve the data; consider actually adding a reference to a datastore object that can do the logic
        void SetDataStore(const std::string& filename, bool compressed, size_t offset, size_t size);

    private:
        std::string _filename;
        bool _compressed;
        size_t _offset, _size;
    };
}
#endif
