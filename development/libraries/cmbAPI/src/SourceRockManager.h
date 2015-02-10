//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file SourceRockManager.h
/// @brief This file keeps API declaration for manipulating source rocks in Cauldron model

#ifndef CMB_SOURCE_ROCK_MANAGER_API
#define CMB_SOURCE_ROCK_MANAGER_API

#include <vector>
#include "ErrorHandler.h"

/// @page SourceRockManagerPage Source Rock Manager
/// @link mbapi::SourceRockManager Source rock manager @endlink provides set of interfaces to create/delete/edit
/// list various source rocks in the data model. Also it has set of interfaces to get/set property of any source
/// rock from the list

namespace mbapi {
   /// @class SourceRockManager SourceRockManager.h "SourceRockManager.h"
   /// @brief Class SourceRockManager keeps a list of source rocks in Cauldron model and allows to add/delete/edit source rock
   class SourceRockManager : public ErrorHandler
   {
   public:
      
      /// @{
      /// Set of interfaces for interacting with a source rock lithologies

      typedef size_t SourceRockID;  ///< unique ID for source rock

      /// @brief Get list of source rocks in the model
      /// @return array with IDs of different source rocks defined in the model
      virtual std::vector<SourceRockID> sourceRockIDs() const = 0; 

      /// @brief Create new source rock lithology record
      /// @return ID of the new source rock lithology
      virtual SourceRockID createNewSourceRockLithology() = 0;

      /// @brief Get layer name for given ID
      /// @param id source rock ID
      /// @return layer name for given source rock lithology ID or empty string in case of error
      virtual std::string layerName( SourceRockID id ) = 0;

      /// @brief Get source rock type name for the given ID
      /// @param id source rock ID
      /// @return source rock type name for the given source rock lithology ID or empty string in case of error
      virtual std::string sourceRockType( SourceRockID id ) = 0;

      /// @}

      /// @{
      /// Source rock TOC (total organic content) API 
      ///
      /// @brief Get total organic contents value ( must be in range 0-100 percent) for the given source rock lithology
      /// @param[in] id source rock ID
      /// @return TOC value for given source rock lithology ID or UndefinedDoubleValue in case of error
      virtual double tocIni( SourceRockID id ) = 0;

      /// @brief Get TOC map name for the given source rock lithology
      /// @param id source rock ID
      /// @return on success TOC map name (if it was defined or empty string otherwise)
      virtual std::string tocInitMapName( SourceRockID id ) = 0;

      /// @brief Set total organic contents value ( must be in range 0-100 percent) for all source
      ///        rock lithologies associated with the given layer
      /// @param layerName name of the layer
      /// @param newTOC new TOC value
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ReturnCode setTOCIni( const std::string & layerName, double newTOC ) = 0;
   
      /// @}

      /// @{
      /// Source rock HI (hydrogen index initial ratio) API 
      ///
      /// @brief Get HI value ( must be in range 0-1000 kg/tonne) for the given source rock lithology
      /// @param[in] id source rock ID
      /// @return HI value for given source rock lithology ID or UndefinedDoubleValue in case of error
      virtual double hiIni( SourceRockID id ) = 0;

      /// @brief Set hydrogen index initial ratio value ( must be in range 0-1000 kg/tonne) for all source
      ///        rock lithologies associated with the given layer
      /// @param layerName name of the layer
      /// @param newHI new HI value
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ReturnCode setHIIni( const std::string & layerName, double newHI ) = 0;

      /// @}


   protected:
      /// @name Constructors/destructor
      /// @{
      /// @brief Constructor which creates an empty model
      SourceRockManager() {;}
      
      /// @brief Destructor, no any actual work is needed here, all is done in the implementation part
      virtual ~SourceRockManager() {;}

   private:
      /// @{
      /// Copy constructor and operator are disabled, use the copyTo method instead
      SourceRockManager( const SourceRockManager & otherSourceRockManager );
      SourceRockManager & operator = ( const SourceRockManager & otherSourceRockManager );
      /// @}
   };
}

#endif // CMB_SOURCE_ROCK_MANAGER_API
