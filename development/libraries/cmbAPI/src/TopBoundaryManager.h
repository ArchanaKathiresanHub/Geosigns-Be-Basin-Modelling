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

#ifndef CMB_TOP_BOUNDARY_MANAGER_API
#define CMB_TOP_BOUNDARY_MANAGER_API

#include <vector>
#include "ErrorHandler.h"

/// @page TopBoundaryManagerPage top boundary Manager
/// @link mbapi::TopBoundaryManager top boundary manager @endlink provides set of interfaces to create/delete/edit list of top boundary model
/// in the data model. Also it has set of interfaces to get/set property of any top boundary model from the list
///

namespace mbapi {
   /// @class TopBoundaryManager TopBoundaryManager.h "TopBoundaryManager.h"
   /// @brief Class TopBoundaryManager keeps functions for getting and setting the ages for the surfaceDepthIoTbl and surfaceTempIoTbl
   class TopBoundaryManager : public ErrorHandler
   {
   public:

	   typedef size_t SurfaceID;  ///< unique ID for surface

	   virtual std::vector<TopBoundaryManager::SurfaceID> getSurfaceDepthIDs() const = 0;
	   virtual std::vector<TopBoundaryManager::SurfaceID> getSurfaceTempIDs() const = 0;

	   // gets the surface age for the given surface id in the SurfaceDepthIoTbl
	   virtual ReturnCode getSurfaceDepthAge(const SurfaceID id, double& surfAge) = 0;
	   // sets the surface age for the given surface id in the SurfaceDepthIoTbl
	   virtual ReturnCode setSurfaceDepthAge(const SurfaceID id, double& surfAge) = 0;

	   // sets the surface age for the given surface id in the SurfaceTempIoTbl
	   virtual ReturnCode getSurfaceTempAge(const SurfaceID id, double& surfAge) = 0;
	   // gets the surface age for the given surface id in the SurfaceTempIoTbl
	   virtual ReturnCode setSurfaceTempAge(const SurfaceID id, double& surfAge) = 0;

   protected:
      /// @{
      /// Constructors/destructor

      /// @brief Constructor which creates an empty model
      TopBoundaryManager() = default;

      /// @brief Destructor, no any actual work is needed here, all is done in the implementation part
      virtual ~TopBoundaryManager() = default;

	  

      /// @}

   private:
      /// @{
      /// Copy constructor and copy operator are disabled
      TopBoundaryManager(const TopBoundaryManager & otherTopBoundaryManager);
      TopBoundaryManager & operator = (const TopBoundaryManager & otherTopBoundaryManager);
      /// @}
   };
}

#endif // CMB_TOP_BOUNDARY_MANAGER_API

