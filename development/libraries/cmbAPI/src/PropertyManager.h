//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PropertyManager.h 
/// @brief This file keeps API which deal with property requests

#ifndef CMB_PROPERTY_MANAGER
#define CMB_PROPERTY_MANAGER

#include "ErrorHandler.h"

namespace mbapi
{
   /// @brief Class which defines API to manage properties requests
   class PropertyManager : public ErrorHandler
   {
   public:
      /// @brief Destructor
      virtual ~PropertyManager() { ; }

      /// @brief Check if properties table has given property in active state. If given property\n
      /// doesn't exist in table it adds corresponded record, if property isn't active - makes it active
      /// @param propName name of the requested property
      /// @param outputPropOption in which layers option should be requested, for example: None/SourceRockOnly/SedimentsOnly...
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode requestPropertyInSnapshots( const std::string & propName, const std::string & outputPropOption = "" ) = 0;

   protected:
      PropertyManager() { ; }

   private:
      PropertyManager( const PropertyManager & );
      PropertyManager & operator = ( const PropertyManager & );
   };
}

#endif // CMB_PROPERTY_MANAGER
