//                                                                      
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ProjectDataManager.h
/// @brief This file keeps API declaration which deal with project level inputs such as modelling mode, project AOI, name, description etc.

#ifndef CMB_PROJECT_DATA_MANAGER_API
#define CMB_PROJECT_DATA_MANAGER_API

#include "ErrorHandler.h"

/// @page ProjectDataManagerPage ProjectData Manager
/// @link mbapi::ProjectDataManager ProjectData manager @endlink provides set of interfaces to interfaces to load/modify/save ProjectIoTbl inputs in project file which maintains the project level data

namespace mbapi {
   /// @class ProjectDataManager ProjectDataManager.h "ProjectDataManager.h" 
   /// @brief Class ProjectDataManager keeps project level inputs in Cauldron model (such as modelling mode, project AOI, name, description etc.) and allows to add/delete/edit those inputs 
   class ProjectDataManager : public ErrorHandler
   {
   public:

      /// @{
      /// @brief Get number of the x-nodes specified in the ProjectIoTbl
      virtual ReturnCode getNumberOfNodesX(int& NodesX) = 0;

      /// @{
      /// @brief Set number of the x-nodes in the ProjectIoTbl
      virtual ReturnCode setNumberOfNodesX(const int& NodesX) = 0;

	  /// @{
      /// @brief Get the values of WindowXMin, WindowsXMax, StepX, WindowYMin, WindowsYMax and StepY specified in the ProjectIoTbl
      virtual ReturnCode getSimulationWindowDetails(int& WindowXMin, int& WindowXMax, int& stepX, int& WindowYMin, int& WindowYMax, int& stepY) = 0;

	  /// @{
	  /// @brief Set the count of WindowXMin and WindowsXMax and StepX in the ProjectIoTbl
	  virtual ReturnCode setSimulationWindowX(const int& WindowXMin, const int& WindowXMax, const int& StepX) = 0;

      /// @{
      /// @brief Set the count of WindowYMin, WindowsYMax and StepY in the ProjectIoTbl
      virtual ReturnCode setSimulationWindowY(const int& WindowYMin, const int& WindowYMax, const int& StepY) = 0;

      /// @{
      /// @brief Get number of the y-nodes specified in the ProjectIoTbl
      virtual ReturnCode getNumberOfNodesY(int& NodesY) = 0;

      /// @{
      /// @brief Set number of the y-nodes in the ProjectIoTbl
      virtual ReturnCode setNumberOfNodesY(const int& NodesY) = 0;

      /// @{
      /// @brief Get grid specing in x-direction specified in the ProjectIoTbl
      virtual ReturnCode getDeltaX(double& deltaX) = 0;

      /// @{
      /// @brief Set grid specing in x-direction in the ProjectIoTbl
      virtual ReturnCode setDeltaX(const double& deltaX) = 0;

      /// @{
      /// @brief Get grid specing in y-direction specified in the ProjectIoTbl
      virtual ReturnCode getDeltaY(double& deltaY) = 0;

      /// @{
      /// @brief Set grid specing in y-direction in the ProjectIoTbl
      virtual ReturnCode setDeltaY(const double& deltaY) = 0;

      /// @{
      /// @brief Get modelling mode specified in the ProjectIoTbl
      virtual ReturnCode getModellingMode(std::string& modellingMode) = 0;

      /// @{
      /// @brief Set modelling mode in the ProjectIoTbl
      virtual ReturnCode setModellingMode(const std::string& modellingMode) = 0;

      /// @{
      /// @brief Get project description specified in the ProjectIoTbl
      virtual ReturnCode getProjectDescription(std::string& description) = 0;

      /// @{
      /// @brief Set project description in the ProjectIoTbl
      virtual ReturnCode setProjectDescription(const std::string& description) = 0;

      /// @}
   protected:
      /// @{
      /// Constructors/destructor

      /// @brief Constructor which creates an empty model
      ProjectDataManager() = default;

      /// @brief Destructor, no any actual work is needed here, all is done in the implementation part
      virtual ~ProjectDataManager() = default;

      /// @}

   private:
      /// @{
      /// Copy constructor and copy operator are disabled
      ProjectDataManager(const ProjectDataManager& otherProjectDataManager);
      ProjectDataManager& operator = (const ProjectDataManager& otherProjectDataManager);
      /// @}
   };
}

#endif // CMB_FRACTURE_PRESSURE_MANAGER_API




