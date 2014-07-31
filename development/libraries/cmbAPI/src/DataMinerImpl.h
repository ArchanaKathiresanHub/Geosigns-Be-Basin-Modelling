//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file DataMinerImpl.h 
/// @brief This file keeps implementation declaration of API which provides access to simulation results

#ifndef CMB_DATA_MINER_IMPL
#define CMB_DATA_MINER_IMPL

#include "DataMiner.h"

#include <memory>

namespace DataAccess
{
   namespace Mining
   {
      class ProjectHandle;
   }
}


namespace mbapi
{
   /// @page DataMinerImplPage Access to the data from simulator calculation results
   /// Cauldron applications could keep simulation results in project file itself and in
   /// set of HDF binary files where data is grouped by time of the simulation. One such
   /// time event is called snapshot. casa::DataMinerImpl allows to load snapshots, switch between
   /// them and provides set of interfaces to access to the data saved in snapshot file

   /// @brief Class which defines API to access data in snapshots
   class DataMinerImpl : public DataMiner 
   {
   public:
      /// @Constructor
      DataMinerImpl();

      /// @brief Destructor
      virtual ~DataMinerImpl() { ; }

   protected:

   private:
      std::auto_ptr<DataAccess::Mining::ProjectHandle> m_proj; // project handle which keeps 

      DataMinerImpl( const DataMinerImpl & );
      DataMinerImpl & operator = ( const DataMinerImpl & );
   };
}

#endif // CMB_DATA_MINER_IMPL
