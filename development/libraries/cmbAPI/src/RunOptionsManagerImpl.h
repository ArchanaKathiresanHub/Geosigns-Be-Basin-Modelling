//                                                                      
// Copyright (C) 2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RunOptionsManagerImpl.h
/// @brief This file keeps API implementation for manipulating RunOptionss in Cauldron model

#ifndef CMB_RUN_OPTIONS_MANAGER_IMPL_API
#define CMB_RUN_OPTIONS_MANAGER_IMPL_API

#include <memory>

#include "ProjectFileHandler.h"

#include "RunOptionsManager.h"
#include <stdexcept>

namespace mbapi {

   // Class RunOptionsManager keeps a list of RunOptionss in Cauldron model and allows to add/delete/edit RunOptions
   class RunOptionsManagerImpl : public RunOptionsManager
   {
   public:
      // Constructors/destructor
      // brief Constructor which creates an RunOptionsManager
      RunOptionsManagerImpl();

      // Destructor
      virtual ~RunOptionsManagerImpl() { ; }

      // Copy operator
      RunOptionsManagerImpl & operator = (const RunOptionsManagerImpl & otherRunOptionsMgr);

      // Set of interfaces for interacting with a Cauldron model

      // Set project database. Reset all
      void setDatabase(database::ProjectFileHandlerPtr pfh);

	  
	  virtual ReturnCode getTemperatureGradient(double & temperatureGradient);
	  virtual ReturnCode setTemperatureGradient(double & temperatureGradient);

	  virtual ReturnCode getVelAlgorithm(std::string & velAlgorithm);
	  virtual ReturnCode setVelAlgorithm(const std::string & velAlgorithm);
	  
	  virtual ReturnCode getVreAlgorithm(std::string & vreAlgorithm);
	  virtual ReturnCode setVreAlgorithm(const std::string & vreAlgorithm);
	  
	  virtual ReturnCode getPTCouplingMode(std::string & PTCouplingMode);
	  virtual ReturnCode setPTCouplingMode(const std::string & PTCouplingMode);

	  virtual ReturnCode getCoupledMode(int & coupledMode);
	  virtual ReturnCode setCoupledMode(const int & coupledMode);

	  virtual ReturnCode getChemicalCompactionAlgorithm(std::string & chemCompactionAlgorithm);
	  virtual ReturnCode setChemicalCompactionAlgorithm(const std::string & chemCompactionAlgorithm);

	  virtual ReturnCode getOptimisationLevel(std::string & optimizationLevel);
	  virtual ReturnCode setOptimisationLevel(const std::string & optimizationLevel);

	  virtual ReturnCode getOptimalTotalTempDiff(double & OTTempDiff);
	  virtual ReturnCode setOptimalTotalTempDiff(const double & OTTempDiff);

	  virtual ReturnCode getOptimalSourceRockTempDiff(double & OSRTempDiff);
	  virtual ReturnCode setOptimalSourceRockTempDiff(const double & OSRTempDiff);

	  virtual ReturnCode getOptimalTotalPresDiff(double & OTPresDiff);
	  virtual ReturnCode setOptimalTotalPresDiff(const double & OTPresDiff);

	  virtual ReturnCode getWorkflowType(std::string & workflow);




   private:
      // Copy constructor is disabled, use the copy operator instead
      RunOptionsManagerImpl(const RunOptionsManager &);

      database::ProjectFileHandlerPtr m_db;          // cauldron project database
      database::Table               * m_runOptionsIoTbl;  // RunOptions Io table
	  static const char * s_runOptionsTableName;
	  static const char * s_vreAlgorithmFieldName;
	  static const char * s_velAlgorithmFieldName;
	  static const char * s_optimisationLevelFieldName;
	  static const char * s_TemperatureGradientFieldName;
	  static const char * s_OTPresDiffFieldName;
	  static const char * s_OTTempDiffFieldName;
	  static const char * s_OSRTempDiffFieldName;
	  static const char * s_PTCouplingModeFieldName;
	  static const char * s_coupledModeFieldName;
	  static const char * s_chemCompactionAlgorithmFieldName;
	  static const char * s_workflowTypeFieldName;
   };
}

#endif // CMB_RUN_OPTIONS_MANAGER_IMPL_API

