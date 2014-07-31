//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file DataMiner.h 
/// @brief This file keeps API which provides access to simulation results

#ifndef CMB_DATA_MINER 
#define CMB_DATA_MINER

#include "ErrorHandler.h"

namespace mbapi
{
   /// @page DataMinerPage Access to the data from simulator calculation results
   /// Cauldron applications could keep simulation results in project file itself and in
   /// set of HDF binary files where data is grouped by time of the simulation. One such
   /// time event is called snapshot. casa::DataMiner allows to load snapshots, switch between
   /// them and provides set of interfaces to access to the data saved in snapshot file

   /// @brief Class which defines API to access data in snapshots
   class DataMiner : public ErrorHandler
   {
   public:
      /// @brief Destructor
      virtual ~DataMiner() { ; }

   protected:
      DataMiner() { ; }

   private:
      DataMiner(const DataMiner&);
      DataMiner & operator = (const DataMiner&);
   };
}

#endif // CMB_DATA_MINER
