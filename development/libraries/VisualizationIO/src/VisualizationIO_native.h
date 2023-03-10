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
#include "DataStore.h"

namespace CauldronIO
{
    struct DataStoreParams;
    class DataStoreLoad;

    /// \brief Map class implementation with "native" retrieve of data - meaning converted to xml/cldrn format
    class MapNative : public SurfaceData
    {
    public:
        /// \brief Constructor defining if this map is cell centered, and its undefined value
		explicit MapNative(const std::shared_ptr<const Geometry2D>& geometry, float minValue = DefaultUndefinedValue, float maxValue = DefaultUndefinedValue);
        ~MapNative();
        
        /// \brief Prefetch any data: load from disk, do not decompress yet
        virtual void prefetch();
        /// \brief Override the retrieve method to load data from datastore
        virtual void retrieve();
        /// \brief Retrieve the data directly into the provided buffer
        virtual void retrieve(const ArrayView<float> &buffer) const override;
        /// \returns a list of HDFinfo holding the data; can be null
        virtual const std::vector < std::shared_ptr<HDFinfo> >& getHDFinfo() { return m_info; }
        /// \brief Set all variables needed to retrieve the data
        void setDataStore(DataStoreParams* params);
        /// \brief Returns the parameters needed for loading this data
        const DataStoreParams* getDataStoreParams() const;
        /// \brief Method to add HDF data to this class
        /// \returns true if all data needed is now ready (prefetch done)
        virtual bool signalNewHDFdata() { return false; }

    private:
        DataStoreParams* m_params;
        DataStoreLoad* m_dataStore;
        std::vector < std::shared_ptr<HDFinfo> > m_info;
    };

    /// \brief Volume class implementation with native retrieve of data
    class VolumeDataNative : public VolumeData
    {
    public:
		explicit VolumeDataNative(const std::shared_ptr<Geometry3D>& geometry, float minValue = DefaultUndefinedValue, float maxValue = DefaultUndefinedValue);
        ~VolumeDataNative();

        /// \brief Prefetch any data: load from disk, do not decompress yet
        virtual void prefetch();
        /// \brief Override the retrieve method to load data from datastore
        virtual void retrieve();
        /// \brief Retrieve the data directly into the provided buffer
        virtual void retrieve(const ArrayView<float> &buffer) const override;
        /// \returns a list of HDFinfo holding the data; can be null
        virtual const std::vector < std::shared_ptr<HDFinfo> >& getHDFinfo() { return m_info; }
        /// \brief Set all variables needed to retrieve the data
        void setDataStore(DataStoreParams* params, bool dataIJK);
        /// \brief Returns the parameters needed for loading this data, IJK
        const DataStoreParams* getDataStoreParamsIJK() const;
        /// \brief Returns the parameters needed for loading this data, KIJ
        const DataStoreParams* getDataStoreParamsKIJ() const;
        /// \brief Method to add HDF data to this class
        /// \returns true if all data needed is now ready (prefetch done)
        virtual bool signalNewHDFdata() { return false; }

    private:
        bool m_dataIJK, m_dataKIJ;
        DataStoreParams* m_paramsIJK;
        DataStoreParams* m_paramsKIJ;
        DataStoreLoad* m_dataStoreIJK;
        DataStoreLoad* m_dataStoreKIJ;
        std::vector < std::shared_ptr<HDFinfo> > m_info;
    };

	/// \brief Volume class implementation with references to other data ONLY
	class ReferenceVolume : public VolumeData
	{
	public:
		explicit ReferenceVolume(const std::shared_ptr<Geometry3D>& geometry, float minValue = DefaultUndefinedValue,
			float maxValue = DefaultUndefinedValue);
		~ReferenceVolume() {}

		/// \brief Prefetch any data: load from disk, do not decompress yet
		virtual void prefetch() {}
		/// \brief Override the retrieve method to load data from datastore
		virtual void retrieve() {}
		/// \brief Retrieve the data directly into the provided buffer
		virtual void retrieve(const ArrayView<float> & /*buffer*/) const override {}
		/// \returns a list of HDFinfo holding the data; can be null
		virtual const std::vector < std::shared_ptr<HDFinfo> >& getHDFinfo() { return m_info; }
		/// \returns true if all data needed is now ready (prefetch done)
		virtual bool signalNewHDFdata() { return false; }
		/// \brief Set all variables needed to retrieve the data
		void setDataStore(const DataStoreParams* params, bool dataIJK);
		/// \brief Returns the parameters needed for loading this data, IJK
		const DataStoreParams* getDataStoreParamsIJK() const;
		/// \brief Returns the parameters needed for loading this data, KIJ
		const DataStoreParams* getDataStoreParamsKIJ() const;

	private:
		std::vector < std::shared_ptr<HDFinfo> > m_info;
		bool m_dataIJK, m_dataKIJ;
		const DataStoreParams* m_paramsIJK;
		const DataStoreParams* m_paramsKIJ;
	};

	/// \brief Map class implementation with reference to other data ONLY
	class ReferenceMap : public SurfaceData
	{
	public:
		/// \brief Constructor defining if this map is cell centered, and its undefined value
		explicit ReferenceMap(const std::shared_ptr<const Geometry2D>& geometry, float minValue = DefaultUndefinedValue, float maxValue = DefaultUndefinedValue);
		~ReferenceMap() {}

		/// \brief Prefetch any data: load from disk, do not decompress yet
		virtual void prefetch() {}
		/// \brief Override the retrieve method to load data from datastore
		virtual void retrieve() {}
		/// \brief Retrieve the data directly into the provided buffer
		virtual void retrieve(const ArrayView<float> &/*buffer*/) const override {}
		/// \returns a list of HDFinfo holding the data; can be null
		virtual const std::vector < std::shared_ptr<HDFinfo> >& getHDFinfo() { return m_info; }
		/// \brief Set all variables needed to retrieve the data
		void setDataStore(const DataStoreParams* params);
		/// \brief Returns the parameters needed for loading this data
		const DataStoreParams* getDataStoreParams() const;
		/// \brief Method to add HDF data to this class
		/// \returns true if all data needed is now ready (prefetch done)
		virtual bool signalNewHDFdata() { return false; }

	private:
		const DataStoreParams* m_params;
		std::vector < std::shared_ptr<HDFinfo> > m_info;
	};
}
#endif
