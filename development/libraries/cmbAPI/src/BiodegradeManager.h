//                                                                      
// Copyright (C) 2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file BiodegradeManager.h
/// @brief This file keeps API which deal with biodegradation constants

#ifndef CMB_BIODEGARDE_MANAGER_API
#define CMB_BIODEGARDE_MANAGER_API

#include "ErrorHandler.h"

namespace mbapi
{
   /// @brief Class which defines API to manage biodegradation constants
   class BiodegradeManager : public ErrorHandler
   {
   public:

      /// @{
      /// Biodegradation constants API 
      /// @brief Get biodegradation constants
      virtual ReturnCode getBioConstant(const std::string & BioConsName, double & BioConsFromP3dFile) = 0;

      /// @brief Set biodegradation constants
      virtual ReturnCode setBioConstant(const std::string & BioConsName, double & BioConsFromP3dFile) = 0;
      /// @}

   protected:
      /// @name Constructors/destructor
      /// @{
      /// @brief Constructor which creates an empty model
      BiodegradeManager() { ; }

      /// @brief Destructor, no any actual work is needed here, all is done in the implementation part
      virtual ~BiodegradeManager() { ; }

   private:
      /// @{
      /// Copy constructor and copy operator are disabled
      BiodegradeManager(const BiodegradeManager & otherBiodegradeManager);
      BiodegradeManager & operator = (const BiodegradeManager & otherBiodegradeManager);
      /// @}
   };
}

#endif // CMB_BIODEGARDE_MANAGER_API


