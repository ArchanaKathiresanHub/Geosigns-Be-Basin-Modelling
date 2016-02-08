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
    class MapNative : public SurfaceData
    {
    public:
        /// \brief Constructor defining if this map is cell centered, and its undefined value
        MapNative(const boost::shared_ptr<const Geometry2D>& geometry);
        ~MapNative();
        
        /// \brief Override the retrieve method to load data from datastore
        virtual void retrieve();
        /// \brief Set all variables needed to retrieve the data
        void setDataStore(DataStoreParams* params);
        /// \brief Returns the parameters needed for loading this data
        const DataStoreParams* getDataStoreParams() const;

    private:
        DataStoreParams* m_params;
    };

    /// \brief Volume class implementation with native retrieve of data
    class VolumeDataNative : public VolumeData
    {
    public:
        VolumeDataNative(const boost::shared_ptr<const Geometry3D>& geometry);
        ~VolumeDataNative();

        /// \brief Override the retrieve method to load data from datastore
        virtual void retrieve();
        /// \brief Set all variables needed to retrieve the data
        void setDataStore(DataStoreParams* params, bool dataIJK);
        /// \brief Returns the parameters needed for loading this data, IJK
        const DataStoreParams* getDataStoreParamsIJK() const;
        /// \brief Returns the parameters needed for loading this data, KIJ
        const DataStoreParams* getDataStoreParamsKIJ() const;

    private:
        bool m_dataIJK, m_dataKIJ;
        DataStoreParams* m_paramsIJK;
        DataStoreParams* m_paramsKIJ;
    };
}
#endif
