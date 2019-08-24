//                                                                      
// Copyright (C) 2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RunOptionsManager.h
/// @brief This file keeps API declaration for manipulating RunOptionsIoTbl in Cauldron model

#ifndef CMB_RUN_OPTIONS_MANAGER_API
#define CMB_RUN_OPTIONS_MANAGER_API

#include "ErrorHandler.h"

/// @page RunOptionsManagerPage RunOptions Manager
/// @link mbapi::RunOptionsManager RunOptions manager @endlink provides set of interfaces to create/delete/edit list RunOptionss
/// in the data model. Also it has set of interfaces to get/set property of any RunOptions from the list
///

namespace mbapi {
   /// @class RunOptionsManager RunOptionsManager.h "RunOptionsManager.h" 
   /// @brief Class RunOptionsManager keeps a list of RunOptionss in Cauldron model and allows to add/delete/edit RunOptions
   class RunOptionsManager : public ErrorHandler
   {
   public:

	   virtual ReturnCode getTemperatureGradient(double & temperatureGradient) = 0;
	   virtual ReturnCode setTemperatureGradient(double & temperatureGradient) = 0;

	   virtual ReturnCode getVelAlgorithm(std::string & velAlgorithm) = 0;
	   virtual ReturnCode setVelAlgorithm(const std::string & velAlgorithm) = 0;

	   virtual ReturnCode getVreAlgorithm(std::string & vreAlgorithm) = 0;
	   virtual ReturnCode setVreAlgorithm(const std::string & vreAlgorithm) = 0;

	   virtual ReturnCode getPTCouplingMode(std::string & PTCouplingMode) = 0;
	   virtual ReturnCode setPTCouplingMode(const std::string & PTCouplingMode) = 0;

	   virtual ReturnCode getCoupledMode(int & coupledMode) = 0;
	   virtual ReturnCode setCoupledMode(const int & coupledMode) = 0;

	   virtual ReturnCode getChemicalCompactionAlgorithm(std::string & chemCompactionAlgorithm) = 0;
	   virtual ReturnCode setChemicalCompactionAlgorithm(const std::string & chemCompactionAlgorithm) = 0;

	   virtual ReturnCode getOptimisationLevel(std::string & optimizationLevel) = 0;
	   virtual ReturnCode setOptimisationLevel(const std::string & optimizationLevel) = 0;

	   virtual ReturnCode getOptimalTotalTempDiff(double & OTTempDiff) = 0;
	   virtual ReturnCode setOptimalTotalTempDiff(const double & OTTempDiff) = 0;

	   virtual ReturnCode getOptimalSourceRockTempDiff(double & OSRTempDiff) = 0;
	   virtual ReturnCode setOptimalSourceRockTempDiff(const double & OSRTempDiff) = 0;

	   virtual ReturnCode getOptimalTotalPresDiff(double & OTPresDiff) = 0;
	   virtual ReturnCode setOptimalTotalPresDiff(const double & OTPresDiff) = 0;

	   virtual ReturnCode getWorkflowType(std::string & workflow) = 0;
   protected:
      /// @{
      /// Constructors/destructor

      /// @brief Constructor which creates an empty model
      RunOptionsManager() = default;

      /// @brief Destructor, no any actual work is needed here, all is done in the implementation part
      virtual ~RunOptionsManager() = default;

      /// @}

   private:
      /// @{
      /// Copy constructor and copy operator are disabled
      RunOptionsManager(const RunOptionsManager & otherRunOptionsManager);
      RunOptionsManager & operator = (const RunOptionsManager & otherRunOptionsManager);
      /// @}
   };
}

#endif // CMB_RUN_OPTIONS_MANAGER_API


