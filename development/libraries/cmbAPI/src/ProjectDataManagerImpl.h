
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
/// @brief This file keeps API declaration which deal with fracture pressure related inputs

#ifndef CMB_PROJECT_DATA_MANAGER_IMPL_API
#define CMB_PROJECT_DATA_MANAGER_IMPL_API

#include <memory>

#include "ProjectFileHandler.h"

#include "ProjectDataManager.h"

namespace mbapi {

   // Class ProjectDataManager keeps fracture pressure inputs in Cauldron model and allows to add / delete / edit those inputs

   class ProjectDataManagerImpl : public ProjectDataManager
   {
   public:
      // Constructors/destructor
      // brief Constructor which creates an ProjectDataManager
      ProjectDataManagerImpl();

      // Destructor
      virtual ~ProjectDataManagerImpl() { ; }

      // Copy operator
      ProjectDataManagerImpl& operator = (const ProjectDataManagerImpl& projectMgr);

      // Set project database. Reset all
      void setDatabase(database::ProjectFileHandlerPtr pfh);

      /// @{
      /// @brief Get number of the x-nodes specified in the ProjectIoTbl
      virtual ReturnCode getNumberOfNodesX(int& NodesX) ;

      /// @{
      /// @brief Set number of the x-nodes in the ProjectIoTbl
      virtual ReturnCode setNumberOfNodesX(const int& NodesX) ;

      /// @{
      /// @brief Get number of the y-nodes specified in the ProjectIoTbl
      virtual ReturnCode getNumberOfNodesY(int& NodesY) ;

      /// @{
      /// @brief Set number of the y-nodes in the ProjectIoTbl
      virtual ReturnCode setNumberOfNodesY(const int& NodesY) ;

      /// @{
      /// @brief Get grid specing in x-direction specified in the ProjectIoTbl
      virtual ReturnCode getDeltaX(double& deltaX) ;

      /// @{
      /// @brief Set grid specing in x-direction in the ProjectIoTbl
      virtual ReturnCode setDeltaX(const double& deltaX) ;

      /// @{
      /// @brief Get grid specing in y-direction specified in the ProjectIoTbl
      virtual ReturnCode getDeltaY(double& deltaY) ;

      /// @{
      /// @brief Set grid specing in y-direction in the ProjectIoTbl
      virtual ReturnCode setDeltaY(const double& deltaY) ;

      /// @{
      /// @brief Get project description specified in the ProjectIoTbl
      virtual ReturnCode getProjectDescription(std::string& description) ;

      /// @{
      /// @brief Set project description in the ProjectIoTbl
      virtual ReturnCode setProjectDescription(const std::string& description) ;

	  /// @{
	  /// @brief Get the values of WindowXMin, WindowsXMax, StepX, WindowYMin, WindowsYMax and StepY specified in the ProjectIoTbl
	  virtual ReturnCode getSimulationWindowDetails(int& WindowXMin, int& WindowXMax, int& stepX, int& WindowYMin, int& WindowYMax, int& stepY);

	  /// @{
	  /// @brief Set the count of WindowXMin, WindowsXMax and StepX in the ProjectIoTbl
	  virtual ReturnCode setSimulationWindowX(const int& WindowXMin, const int& WindowXMax, const int& StepX);

      /// @{
      /// @brief Set the count of WindowYMin, WindowsYMax and StepY in the ProjectIoTbl
      virtual ReturnCode setSimulationWindowY(const int& WindowYMin, const int& WindowYMax, const int& StepY);

   private:
      // Copy constructor is disabled, use the copy operator instead
      ProjectDataManagerImpl(const ProjectDataManager&);

      static const char* s_projectOptionsTableName;
      
      database::ProjectFileHandlerPtr m_db;  // cauldron project database
      database::Table* m_projectIoTbl;       // ProjectIo table
      
      //static const char* s_xcoordFieldName;  // column name for x-coordinate of origin in ProjectIoTbl
      //static const char* s_ycoordFieldName;  // column name for y-coordinate of origin in ProjectIoTbl
      static const char* s_xNodesFieldName;  // column name for number of nodes in x direction
      static const char* s_yNodesFieldName;  // column name for number of nodes in y direction
      static const char* s_xGridSpecingFieldName;  // column name for grid specing in x direction
      static const char* s_yGridSpecingFieldName;  // column name for grid specing in y direction      
      static const char* s_projectDescriptionFieldName;  // column name for project description
      static const char* s_windowXMaxFieldName;  // column name for max window in x-direction
      static const char* s_windowXMinFieldName;  // column name for min window in x-direction
      static const char* s_windowYMaxFieldName;  // column name for max window in y-direction
      static const char* s_windowYMinFieldName;  // column name for min window in y-direction
      static const char* s_stepXFieldName;  // column name for subsampled size in x-direction
      static const char* s_stepYFieldName;  // column name for subsampled size in y-direction      
   };
}

#endif // CMB_PROJECT_DATA_MANAGER_IMPL_API


