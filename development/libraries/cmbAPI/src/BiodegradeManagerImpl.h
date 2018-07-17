#pragma once
//                                                                      
// Copyright (C) 2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file BiodegradeManagerImpl.h
/// @brief This file keeps API which deal with biodegradation constants

#ifndef CMB_BIODEGARDE_MANAGER_IMPL_API
#define CMB_BIODEGARDE_MANAGER_IMPL_API

#include <memory>

#include "ProjectFileHandler.h"

#include "BiodegradeManager.h"

namespace mbapi {

   // Class BiodegradeManager keeps a boidegradation constants
   class BiodegradeManagerImpl : public BiodegradeManager
   {
   public:
      // Constructors/destructor
      // brief Constructor which creates an FluidManager
      BiodegradeManagerImpl();

      // Destructor
      virtual ~BiodegradeManagerImpl() { ; }

      // Copy operator
      BiodegradeManagerImpl & operator = (const BiodegradeManagerImpl & otherBiodegradeManager);

      // Set project database. Reset all
      void setDatabase(database::ProjectFileHandlerPtr pfh);

      /// @{
      /// Biodegradation constants API 
      /// @brief Get biodegradation constants
      virtual ReturnCode getBioConstant(const std::string & BioConsName, double & BioConsFromP3dFile);

      /// @brief Set biodegradation constants
      virtual ReturnCode setBioConstant(const std::string & BioConsName, double & BioConsFromP3dFile);
      /// @}


   private:
      // Copy constructor is disabled, use the copy operator instead
      BiodegradeManagerImpl(const BiodegradeManagerImpl &);

      static const char * s_BiodegradeTableName;
      database::ProjectFileHandlerPtr m_db;          // cauldron project database
      database::Table * m_BiodegradeIoTbl;  // Biodegradation Io table

   };
}

#endif // CMB_BIODEGARDE_MANAGER_IMPL_API