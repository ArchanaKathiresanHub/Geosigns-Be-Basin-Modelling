//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file SnapshotManager.h 
/// @brief This file keeps API which provides access to snapshots in project file 

#ifndef CMB_SNAPSHOT_MANAGER
#define CMB_SNAPSHOT_MANAGER

#include "ErrorHandler.h"

namespace mbapi
{
   /// @brief Class which defines API to manage snapshots in project file
   class SnapshotManager : public ErrorHandler
   {
   public:
      /// @brief Destructor
      virtual ~SnapshotManager() {}

      /// @brief Get number of snapshots in project
      /// @return number of snapshots 
      virtual size_t snapshotsNumber() const = 0;

      /// @brief Get time for the i-th snapshot
      /// @param i snapshot number
      /// @return time for the i-th snapshot
      virtual double time( size_t i ) const = 0;

      /// @brief Ask, is i-th snapshot minor?
      /// @param i snapshot number
      /// @return true if it is minor snapshot, false otherwise for 
      virtual bool isMinor( size_t i ) const = 0;

      /// @brief Get snapshot type
      /// @param i snapshot number
      /// @return type of i-th snapshot
      virtual std::string type( size_t i ) = 0;

      /// @brief Get data file name for i-th snapshot
      /// @param i snapshot number
      /// @return name of the data file for i-th snapshot if it exists, or empty string if it is not
      virtual std::string fileName( size_t i ) = 0;

      /// @brief Check if major snapshot exists for the given time, and if not, create one
      /// @param simTime time for snapshot
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode requestMajorSnapshot( double simTime ) = 0;

      /// @brief Get all major snapshots
      /// @return All major snapshots
      virtual std::vector<double> agesFromMajorSnapshots() const = 0;

   protected:
      SnapshotManager() {}

   private:
      SnapshotManager(const SnapshotManager&);
      SnapshotManager & operator = (const SnapshotManager&);
   };
}

#endif // CMB_SNAPSHOT_MANAGER
