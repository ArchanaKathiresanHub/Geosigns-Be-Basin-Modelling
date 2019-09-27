//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file SnapshotManagerImpl.h
/// @brief This file keeps implementation declarations for API which provides access to simulation results

#ifndef CMB_SNAPSHOT_MANAGER_IMPL
#define CMB_SNAPSHOT_MANAGER_IMPL

#include "ProjectFileHandler.h"

#include "SnapshotManager.h"

#include <string>

namespace database
{
   class Database;
   class Table;
}

namespace mbapi
{
   /// @brief Class which defines implementation of API to access data in snapshots
   class SnapshotManagerImpl : public SnapshotManager
   {
   public:
      // Constructor
      SnapshotManagerImpl();

      /// @brief Destructor
      virtual ~SnapshotManagerImpl() {}

      // Set project database. Reset all
      void setDatabase( database::ProjectFileHandlerPtr pfh, const std::string & projName );


      /// @brief Get number of snapshots in project
      /// @return number of snapshots
      virtual size_t snapshotsNumber() const;

      /// @brief Get time for the i-th snapshot
      /// @param i snapshot number
      /// @return time for the i-th snapshot
      virtual double time( size_t i ) const;

      /// @brief Ask, is i-th snapshot minor?
      /// @param i snapshot number
      /// @return true if it is minor snapshot, false otherwise for
      virtual bool isMinor( size_t i ) const;

      /// @brief Get snapshot type
      /// @param i snapshot number
      /// @return type of i-th snapshot
      virtual std::string type( size_t i );

      /// @brief Get data file name for i-th snapshot
      /// @param i snapshot number
      /// @return name of the data file for i-th snapshot if it exists, or empty string if it is not
      virtual std::string fileName( size_t i );

      /// @brief Check if major snapshot exists for the given time, and if not, create one
      /// @param simTime time for snapshot
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode requestMajorSnapshot( double simTime );

      /// @brief Get all major snapshots
      /// @return All major snapshots
      virtual std::vector<double> agesFromMajorSnapshots() const;

   protected:

   private:
      static double                                     s_snpTol;   // small value for comparison of snapshot time
      database::ProjectFileHandlerPtr                   m_db;       // project database
      database::Table                                 * m_snpTable; // snapshot table

      SnapshotManagerImpl( const SnapshotManagerImpl & );
      SnapshotManagerImpl & operator = ( const SnapshotManagerImpl & );
   };
}

#endif // CMB_SNAPSHOT_MANAGER_IMPL
