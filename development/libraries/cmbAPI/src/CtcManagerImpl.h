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
/// @brief This file keeps API declaration for manipulating CTC inputs in Cauldron model

#ifndef CMB_CTC_MANAGER_IMPL_API
#define CMB_CTC_MANAGER_IMPL_API

#include <memory>

#include "ProjectFileHandler.h"

#include "CtcManager.h"

namespace mbapi {

   // Class CtcManager keeps a list of inputs for CTC in Cauldron model and allows to add/delete/edit those
   class CtcManagerImpl : public CtcManager
   {
   public:
      // Constructors/destructor
      // brief Constructor which creates an CtcManager
      CtcManagerImpl();

      // Destructor
      virtual ~CtcManagerImpl() { ; }

      // Copy operator
      CtcManagerImpl & operator = (const CtcManagerImpl & otherFluidMgr);

     

      // Set project database. Reset all
      void setDatabase(database::ProjectFileHandlerPtr pfh);

      /// @{
      /// @brief Get filter half width value
      virtual ReturnCode getFilterHalfWidthValue(int & FiltrHalfWidth);

      /// @{
      /// @brief Set filter half width value
      virtual ReturnCode setFilterHalfWidthValue(int & FiltrHalfWidth);

      /// @{
      /// @brief Get Upper Lower Continental Crust Ratio value
      virtual ReturnCode getUpperLowerContinentalCrustRatio(double & ULContCrustRatio);
      
      /// @{
      /// @brief Set UpperLowerContinentalCrustRatio value
      virtual ReturnCode setUpperLowerContinentalCrustRatio(double & ULContCrustRatio);

      /// @{
      /// @brief Get UpperLowerOceanicCrustRatio value
      virtual ReturnCode getUpperLowerOceanicCrustRatio(double & ULOceanicCrustRatio);

      /// @{
      /// @brief Set UpperLowerOceanicCrustRatio value
      virtual ReturnCode setUpperLowerOceanicCrustRatio(double & ULOceanicCrustRatio);

   private:
      // Copy constructor is disabled, use the copy operator instead
      CtcManagerImpl(const CtcManager &);

      static const char * s_ctcTableName;

      database::ProjectFileHandlerPtr m_db;                // cauldron project database
      database::Table               * m_ctcIoTbl;          // CTC Io table

      static const char * s_FilterHalfWidthFieldName;  // column name for filter half width field
      static const char * s_ULContCrustRatioFieldName;  // column name for filter half width field             
      static const char * s_ULOceaCrustRatioFieldName;  // column name for filter half width field 


   };
}

#endif // CMB_CTC_MANAGER_IMPL_API

