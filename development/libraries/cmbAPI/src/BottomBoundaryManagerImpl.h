//                                                                      
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file BottomBoundaryManager.h
/// @brief This file keeps API declaration for manipulating bottom boundary model in Cauldron model

#ifndef CMB_BOTTOM_BOUNDARY_MANAGER_IMPL_API
#define CMB_BOTTOM_BOUNDARY_MANAGER_IMPL_API

#include <memory>

#include "ProjectFileHandler.h"

#include "BottomBoundaryManager.h"

namespace mbapi {

   // Class BottomBoundaryManager keeps a list of bottom boundary models in Cauldron model and allows to add/delete/edit those
   class BottomBoundaryManagerImpl : public BottomBoundaryManager
   {
   public:
      // Constructors/destructor
      // brief Constructor which creates an BottomBoundaryManager
      BottomBoundaryManagerImpl();

      // Destructor
      virtual ~BottomBoundaryManagerImpl() { ; }

      // Copy operator
      BottomBoundaryManagerImpl & operator = (const BottomBoundaryManagerImpl & otherFluidMgr);

      // Set of interfaces for interacting with a Cauldron model

      // Set project database. Reset all
      void setDatabase(database::ProjectFileHandlerPtr pfh);


      /// @{
      /// @brief Get bottom boundary model name
      virtual ReturnCode getBottomBoundaryModel(BottomBoundaryModel & BBCModel);

      /// @brief Set bottom boundary model name
      virtual ReturnCode setBottomBoundaryModel(const BottomBoundaryModel & BBCModel);
      /// @}

      /// @{
      /// @brief Get property model for crust
      virtual ReturnCode getCrustPropertyModel(CrustPropertyModel & CrustPropModel);

      /// @{
      /// @brief Set property model for crust
      virtual ReturnCode setCrustPropertyModel(const CrustPropertyModel & CrustPropModel);

      /// @{
      /// @brief Get mantle model for mantle
      virtual ReturnCode getMantlePropertyModel(MantlePropertyModel & MantlePropModel);

      /// @{
      /// @brief Set property model for mantle
      virtual ReturnCode setMantlePropertyModel(const MantlePropertyModel & MantlePropModel);

      /// @{
      /// @brief Get initial lithospheric mantle thickness value
      virtual ReturnCode getInitialLithoMantleThicknessValue(double & IniLthMantThic);

      /// @{
      /// @brief Set initial lithpspheric mantle thickness value
      virtual ReturnCode setInitialLithoMantleThicknessValue(double & IniLthMantThic);

      /// @brief Get list of time steps referred in the crustal thickness in the CrustIoTbl
      /// @return array with IDs of different time steps referred in the crustal thickness in the CrustIoTbl
      virtual std::vector<TimeStepID> getTimeStepsID() const;

      /// @brief Get age from the CrustIoTbl
      /// @param[in] id time step ID
      /// @param[out] Age specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getAge(const TimeStepID id, double & Age);

      /// @brief Set age in the CrustIoTbl
      /// @param[in] id time step ID
      /// @param[in] Age specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setAge(const TimeStepID id, const double Age);

      /// @brief Get thickness value from the CrustIoTbl
      /// @param[in] id time step ID
      /// @param[out] Thickness value specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getThickness(const TimeStepID id, double & Thickness);

      /// @brief Set thickness value in the CrustIoTbl
      /// @param[in] id time step ID
      /// @param[in] Thickness value specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setThickness(const TimeStepID id, const double Thickness);

      /// @brief Get crust thickness map name from the CrustIoTbl for
      /// @param[in] id time step ID
      /// @param[out] mapName specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getCrustThicknessGrid(const TimeStepID id, std::string & mapName);

      /// @brief Set crust thickness map name in CrustIoTbl for 
      /// @param[in] id time step ID
      /// @param[in] mapName specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setCrustThicknessGrid(const TimeStepID id, const std::string & mapName);

      /// @brief Get list of time steps used in the continental crustal thickness Io table
      /// @return array with IDs of different time steps used in the continental crustal thickness Io table
      virtual std::vector<ContCrustTimeStepID> getContCrustTimeStepsID() const;

      /// @brief Get age from the continental crustal thickness Io table for 
      /// @param[in] id time step ID
      /// @param[out] Age specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getContCrustAge(const ContCrustTimeStepID id, double & Age);

      /// @brief Set age in the continental crustal thickness table for 
      /// @param[in] id time step ID
      /// @param[in] Age specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setContCrustAge(const ContCrustTimeStepID id, const double Age);

      /// @brief Get continental crust thickness value from the continental crustal thickness Io table for 
      /// @param[in] id time step ID
      /// @param[out] Thikness value specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getContCrustThickness(const ContCrustTimeStepID id, double & Thikness);

      /// @brief Set continental crust thickness value in the continental crustal thickness Io table for 
      /// @param[in] id time step ID
      /// @param[in] Thikness value specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setContCrustThickness(const ContCrustTimeStepID id, const double Thikness);

      /// @brief Get continental crust thickness map name from the continental crustal thickness Io table for
      /// @param[in] id time step ID
      /// @param[out] mapName specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getContCrustThicknessGrid(const ContCrustTimeStepID id, std::string & mapName);

      /// @brief Set continental crust thickness grid in the continental crustal thickness Io table for 
      /// @param[in] id time step ID
      /// @param[in] mapName specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setContCrustThicknessGrid(const ContCrustTimeStepID id, const std::string & mapName);

      /// @brief Get list of time steps used in the oceanic crustal thickness Io table
      /// @return array with IDs of different time steps used in the oceanic crustal thickness table
      virtual std::vector<OceaCrustTimeStepID> getOceaCrustTimeStepsID() const;

      /// @brief Get age from oceanic crustal thickness Io table for 
      /// @param[in] id time step ID
      /// @param[out] Age specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getOceaCrustAge(const OceaCrustTimeStepID id, double & Age);

      /// @brief Set age in oceanic crustal thickness Io table for 
      /// @param[in] id time step ID
      /// @param[in] Age specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setOceaCrustAge(const OceaCrustTimeStepID id, const double Age);

      /// @brief Get oceanic crust thickness value from oceanic crustal thickness Io table for
      /// @param[in] id time step ID
      /// @param[out] Thikness value specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getOceaCrustThickness(const OceaCrustTimeStepID id, double & Thikness);

      /// @brief Set oceanic crust thickness value in oceanic crustal thickness Io table for 
      /// @param[in] id time step ID
      /// @param[in] Thikness value specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setOceaCrustThickness(const OceaCrustTimeStepID id, const double Thikness);

      /// @brief Get list of time steps used in the GridMapIoTbl 
      /// @return array with IDs of different time steps used in the GridMapIoTbl
      virtual std::vector<GridMapTimeStepID> getGridMapTimeStepsID() const;

      /// @brief Get "ReferredBy" field from GridMapIoTbl for
      /// @param[in] id time step ID
      /// @param[out] mapName specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getReferredBy(const GridMapTimeStepID id, std::string & mapName);

      /// @brief Set ReferredBy field in GridMapIoTbl for 
      /// @param[in] id time step ID
      /// @param[in] mapName specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setReferredBy(const GridMapTimeStepID id, const std::string & mapName);


   private:
      // Copy constructor is disabled, use the copy operator instead
      BottomBoundaryManagerImpl(const BottomBoundaryManager &);

      static const char * s_bottomBoundaryModelTableName;
      static const char * s_crustalThicknessTableName;
      static const char * s_contCrustalThicknessTableName;
      static const char * s_oceaCrustalThicknessTableName;
      static const char * s_gridMapTableName;
      database::ProjectFileHandlerPtr m_db;                // cauldron project database
      database::Table               * m_basementIoTbl;     // Basement Type Io table
      database::Table               * m_crustIoTbl;        // crust Io table
      database::Table               * m_contCrustIoTbl;    // continental crust Io table
      database::Table               * m_oceaCrustIoTbl;    // oceanic crust Io table
      database::Table               * m_gridMapIoTbl;      // grid map Io table
      static const char * s_bottomBoundaryModelFieldName;  // column name for bottom boundary model
      static const char * s_crustPropModelFieldName;       // column name for crust property model
      static const char * s_mantlePropModelFieldName;		  // column name for mantle property model name
      static const char * s_initLithManThicknessFieldName;	// column name for initial lithospheric mantle thickness
      static const char * s_CrustalThicknessAgeFieldName;		// column name for crust Age value in CrustIoTbl
      static const char * s_CrustalThicknessThicknessFieldName;	// column name for crust thickness value in CrustIoTbl
      static const char * s_CrustalThicknessGridFieldName;	// column name for crustal thickness grid map in CrustIoTbl
      static const char * s_ContCrustalThicknessAgeFieldName;		// column name for continental crust Age value in ContCrustalThicknessIoTbl
      static const char * s_ContCrustalThicknessFieldName;	// column name for continental crust thickness value in ContCrustalThicknessIoTbl
      static const char * s_ContCrustalThicknessGridFieldName;	// column name for continental crustal thickness grid map in ContCrustalThicknessIoTbl
      static const char * s_OceaCrustalThicknessAgeFieldName;		// column name for oceanic crust Age value in OceaCrustIoTbl
      static const char * s_OceaCrustalThicknessFieldName;	// column name for oceanic crustal thickness value in OceaCrustIoTbl
      static const char * s_GridMapRefByFieldName;	// column name for grid map "ReferredBy" field of GridMapIoTbl


   };
}

#endif // CMB_BOTTOM_BOUNDARY_MANAGER_IMPL_API
