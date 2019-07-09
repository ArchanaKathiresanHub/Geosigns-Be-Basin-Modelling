//                                                                      
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file TopBoundaryManager.h
/// @brief This file keeps API declaration for manipulating top boundary model in Cauldron model

#ifndef CMB_TOP_BOUNDARY_MANAGER_IMPL_API
#define CMB_TOP_BOUNDARY_MANAGER_IMPL_API

#include <memory>

#include "ProjectFileHandler.h"

#include "TopBoundaryManager.h"

namespace mbapi {

   // Class TopBoundaryManager keeps a list of top boundary models in Cauldron model and allows to add/delete/edit those
   class TopBoundaryManagerImpl : public TopBoundaryManager
   {
   public:
      // Constructors/destructor
      // brief Constructor which creates an TopBoundaryManager
      TopBoundaryManagerImpl();

      // Destructor
      virtual ~TopBoundaryManagerImpl() { ; }

      // Copy operator
      TopBoundaryManagerImpl & operator = (const TopBoundaryManagerImpl & otherBoundaryManager);


      // Set project database. Reset all
      void setDatabase(database::ProjectFileHandlerPtr pfh);

	  std::vector<TopBoundaryManager::SurfaceID>  getSurfaceDepthIDs() const;
	  std::vector<TopBoundaryManager::SurfaceID>  getSurfaceTempIDs() const;
	  virtual ReturnCode getSurfaceDepthAge(const SurfaceID id, double& surfAge);
	  virtual ReturnCode setSurfaceDepthAge(const SurfaceID id, double& surfAge);

	  virtual ReturnCode getSurfaceTempAge(const SurfaceID id, double& surfAge);
	  virtual ReturnCode setSurfaceTempAge(const SurfaceID id, double& surfAge);


   private:
      // Copy constructor is disabled, use the copy operator instead
      TopBoundaryManagerImpl(const TopBoundaryManager &);

      static const char * s_surfaceDepthIoTableName;
      static const char * s_surfceDepthAgeFieldName;
      static const char * s_surfaceTempIoTableName;
      static const char * s_surfceTempAgeFieldName;
      database::ProjectFileHandlerPtr m_db;					// cauldron project database
      database::Table               * m_surfaceDepthIoTbl;  // surface depth Io table
      database::Table               * m_surfaceTempIoTbl;   // surface temp Io table
      
      

   };
}

#endif // CMB_TOP_BOUNDARY_MANAGER_IMPL_API
