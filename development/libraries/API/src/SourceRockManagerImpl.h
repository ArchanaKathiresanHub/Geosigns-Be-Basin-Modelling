//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file SourceRockManagerImpl.h
/// @brief This file keeps API implementation for manipulating source rocks in Cauldron model

#ifndef CMB_SOURCE_ROCK_MANAGER_IMPL_API
#define CMB_SOURCE_ROCK_MANAGER_IMPL_API

#include "SourceRockManager.h"

namespace database
{
   class Database;
}

namespace mbapi {

   // Class SourceRockManager keeps a list of source rocks in Cauldron model and allows to add/delete/edit source rock
   class SourceRockManagerImpl : public SourceRockManager
   {
   public:     
      // Constructors/destructor
      // brief Constructor which creates an SourceRockManager
      SourceRockManagerImpl();
      
      // Destructor
      virtual ~SourceRockManagerImpl() {;}

      // Copy operator
      SourceRockManagerImpl & operator = ( const SourceRockManagerImpl & otherSrRockMgr );

      // Set of interfaces for interacting with a Cauldron model
      
      // Set project database. Reset all
      void setDatabase( database::Database * db );

      // Get list of source rocks in the model
      // return array with IDs of different lithologies defined in the model
      virtual std::vector<SourceRockID> sourceRockIDs( ) const;
      // Create new source rock

      // return ID of the new source rock lithology
      virtual SourceRockID createNewSourceRockLithology( );

      // Get layer name for given ID
      virtual std::string layerName( SourceRockID id );

      // Get source rock type name for the given ID
      virtual std::string sourceRockType( SourceRockID id );

      // Get total organic contents value ( must be in range 0-100 percent) for the given source rock lithology
      virtual double tocIni( SourceRockID id );

      // Get TOC map name for the given source rock lithology
      virtual std::string tocInitMapName( SourceRockID id );

      // Set total organic contents value ( must be in range 0-100 percent) for all source
      // rock lithologies associated with the given layer
      virtual ReturnCode setTOCIni( const std::string & layerName, double newTOC );

   private:
      static const char * m_sourceRockTableName;     // table name for source rock lithologies in project file
      static const char * m_layerNameFieldName;      // name of the field which keeps layer name
      static const char * m_sourceRockTypeFieldName; // name of the field which keeps source rock type name
      static const char * m_tocIni;                  // initial TOC
      static const char * m_tocIniMap;               // initial TOC map name

      // Copy constructor is disabled, use the copy operator instead
      SourceRockManagerImpl( const SourceRockManager & );

      database::Database * m_db; // cauldron project database
   };
}

#endif // CMB_SOURCE_ROCK_MANAGER_IMPL_API