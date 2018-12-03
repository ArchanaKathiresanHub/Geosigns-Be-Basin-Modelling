//                                                                      
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file CtcManager.h
/// @brief This file keeps API declaration for manipulating CTCv1 inputs as required in CTCv2 in Cauldron model

#ifndef CMB_CTC_MANAGER_API
#define CMB_CTC_MANAGER_API

#include "ErrorHandler.h"

/// @page CtcManagerPage CTC Manager
/// @link mbapi::CtcManager CTC manager @endlink provides set of interfaces to create/delete/edit list of CTC inputs
/// in the data model. Also it has a set of interfaces to get/set property of any CTC inputs from CTC related Io tables
///

namespace mbapi {
   /// @class CtcManager CtcManager.h "CtcManager.h" 
   /// @brief Class CtcManager keeps a list of CTC inputs in Cauldron model and allows to add/delete/edit those inputs 
   class CtcManager : public ErrorHandler
   {
   public:

      /// @{
      /// @brief Get FilterHalfWidth value
      virtual ReturnCode getFilterHalfWidthValue(int & FiltrHalfWidth) = 0;

      /// @{
      /// @brief Set FilterHalfWidth value
      virtual ReturnCode setFilterHalfWidthValue(int & FiltrHalfWidth) = 0;

      /// @{
      /// @brief Get UpperLowerContinentalCrustRatio value
      virtual ReturnCode getUpperLowerContinentalCrustRatio(double & ULContCrustRatio) = 0;

      /// @{
      /// @brief Set UpperLowerContinentalCrustRatio value
      virtual ReturnCode setUpperLowerContinentalCrustRatio(double & ULContCrustRatio) = 0;

      /// @{
      /// @brief Get UpperLowerOceanicCrustRatio value
      virtual ReturnCode getUpperLowerOceanicCrustRatio(double & ULOceanicCrustRatio) = 0;

      /// @{
      /// @brief Set UpperLowerOceanicCrustRatio value
      virtual ReturnCode setUpperLowerOceanicCrustRatio(double & ULOceanicCrustRatio) = 0;


      /// @}
   protected:
      /// @{
      /// Constructors/destructor

      /// @brief Constructor which creates an empty model
      CtcManager() = default;

      /// @brief Destructor, no any actual work is needed here, all is done in the implementation part
      virtual ~CtcManager() = default;

      /// @}

   private:
      /// @{
      /// Copy constructor and copy operator are disabled
      CtcManager(const CtcManager & otherCtcManager);
      CtcManager & operator = (const CtcManager & otherCtcManager);
      /// @}
   };
}

#endif // CMB_CTC_MANAGER_API


