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

#ifndef CMB_BOTTOM_BOUNDARY_MANAGER_API
#define CMB_BOTTOM_BOUNDARY_MANAGER_API

#include <vector>
#include "ErrorHandler.h"

/// @page BottomBoundaryManagerPage bottom boundary Manager
/// @link mbapi::BottomBoundaryManager bottom boundary manager @endlink provides set of interfaces to create/delete/edit list of bottom boundary model
/// in the data model. Also it has set of interfaces to get/set property of any bottom boundary model from the list
///

namespace mbapi {
   /// @class BottomBoundaryManager BottomBoundaryManager.h "BottomBoundaryManager.h"
   /// @brief Class BottomBoundaryManager keeps a list of bottom boundary models in Cauldron model and allows to add/delete/edit bottom boundary models and the associated properties
   class BottomBoundaryManager : public ErrorHandler
   {
   public:

      /// @{
      /// Set of interfaces for interacting with a Cauldron model
      typedef size_t TimeStepID;  ///< unique ID for time steps defined in the crustal thickness table
      typedef size_t ContCrustTimeStepID;  ///< unique ID for time steps defined in the continental crustal thickness table
      typedef size_t OceaCrustTimeStepID;  ///< unique ID for time steps defined in the oceanic crustal thickness table
      typedef size_t GridMapTimeStepID;  ///< unique ID for time steps defined in the GridMapIoTbl

      enum BottomBoundaryModel
      {
         BasicCrustThinning,
         AdvancedCrustThinning,
         ImprovedCrustThinningLinear,
         BaseSedimentHeatFlow
      };

      enum CrustPropertyModel
      {
         LegacyCrust,
         LowCondModel,
         StandardCondModel,
         HighCondModel
      };

      enum MantlePropertyModel
      {
         LegacyMantle,
         LowCondMnModel,
         StandardCondMnModel,
         HighCondMnModel
      };


      /// @{
      /// @brief Get bottom boundary model name
      virtual ReturnCode getBottomBoundaryModel(BottomBoundaryModel & BBCModel) = 0;

      /// @brief Set bottom boundary model name
      virtual ReturnCode setBottomBoundaryModel(const BottomBoundaryModel & BBCModel) = 0;
      /// @}

      /// @{
      /// @brief Get property model for crust
      virtual ReturnCode getCrustPropertyModel(CrustPropertyModel & CrustPropModel) = 0;

      /// @{
      /// @brief Set property model for crust
      virtual ReturnCode setCrustPropertyModel(const CrustPropertyModel & CrustPropModel) = 0;

      /// @{
      /// @brief Get property model for crust
      virtual ReturnCode getMantlePropertyModel(MantlePropertyModel & MantlePropModel) = 0;

      /// @{
      /// @brief Set property model for mantle
      virtual ReturnCode setMantlePropertyModel(const MantlePropertyModel & MantlePropModel) = 0;

      /// @{
      /// @brief Get initial lithpspheric mantle thickness value
      virtual ReturnCode getInitialLithoMantleThicknessValue(double & IniLthMantThic) = 0;

      /// @{
      /// @brief Set initial lithpspheric mantle thickness value
      virtual ReturnCode setInitialLithoMantleThicknessValue(double & IniLthMantThic) = 0;

      /// @brief Get list of time steps referred in the crustal thickness in the CrustIoTbl
      /// @return array with IDs of different time steps referred in the crustal thickness in the CrustIoTbl
      virtual std::vector<TimeStepID> getTimeStepsID() const = 0;

      /// @brief Get age from the CrustIoTbl
      /// @param[in] id time step ID
      /// @param[out] age specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getAge(const TimeStepID id, double & Age) = 0;

      /// @brief Set age in the CrustIoTbl
      /// @param[in] id time step ID
      /// @param[in] age specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setAge(const TimeStepID id, const double Age) = 0;

      /// @brief Get thickness value from the CrustIoTbl
      /// @param[in] id time step ID
      /// @param[out] value specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getThickness(const TimeStepID id, double & Thickness) = 0;

      /// @brief Set thickness value in the CrustIoTbl
      /// @param[in] id time step ID
      /// @param[in] value specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setThickness(const TimeStepID id, const double Thicknesse) = 0;

      /// @brief Get crust thickness map name from the CrustIoTbl for
      /// @param[in] id time step ID
      /// @param[out] map name specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getCrustThicknessGrid(const TimeStepID id, std::string & mapName) = 0;

      /// @brief Set crust thickness map name in CrustIoTbl for
      /// @param[in] id time step ID
      /// @param[in] map name specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setCrustThicknessGrid(const TimeStepID id, const std::string & mapName) = 0;

      /// @brief Get list of time steps used in the continental crustal thickness Io table
      /// @return array with IDs of different time steps used in the continental crustal thickness Io table
      virtual std::vector<ContCrustTimeStepID> getContCrustTimeStepsID() const = 0;

      /// @brief Get age from the continental crustal thickness Io table for
      /// @param[in] id time step ID
      /// @param[out] age specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getContCrustAge(const ContCrustTimeStepID id, double & Age) = 0;

      /// @brief Set age in the continental crustal thickness table for
      /// @param[in] id time step ID
      /// @param[in] age specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setContCrustAge(const ContCrustTimeStepID id, const double Age) = 0;

      /// @brief Get continental crust thickness value from the continental crustal thickness Io table for
      /// @param[in] id time step ID
      /// @param[out] value specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getContCrustThickness(const ContCrustTimeStepID id, double & Thikness) = 0;

      /// @brief Set continental crust thickness value in the continental crustal thickness Io table for
      /// @param[in] id time step ID
      /// @param[in] value specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setContCrustThickness(const ContCrustTimeStepID id, const double Thikness) = 0;

      /// @brief Get continental crust thickness map name from the continental crustal thickness Io table for
      /// @param[in] id time step ID
      /// @param[out] map name specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getContCrustThicknessGrid(const ContCrustTimeStepID id, std::string & mapName) = 0;

      /// @brief Set continental crust thickness grid in the continental crustal thickness Io table for
      /// @param[in] id time step ID
      /// @param[in] map name specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setContCrustThicknessGrid(const ContCrustTimeStepID id, const std::string & mapName) = 0;

      /// @brief Get list of time steps used in the oceanic crustal thickness Io table
      /// @return array with IDs of different time steps used in the oceanic crustal thickness table
      virtual std::vector<OceaCrustTimeStepID> getOceaCrustTimeStepsID() const = 0;

      /// @brief Get age from oceanic crustal thickness Io table for
      /// @param[in] id time step ID
      /// @param[out] age specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getOceaCrustAge(const OceaCrustTimeStepID id, double & Age) = 0;

      /// @brief Set age in oceanic crustal thickness Io table for
      /// @param[in] id time step ID
      /// @param[in] age specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setOceaCrustAge(const OceaCrustTimeStepID id, const double Age) = 0;

      /// @brief Get oceanic crust thickness value from oceanic crustal thickness Io table for
      /// @param[in] id time step ID
      /// @param[out] value specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getOceaCrustThickness(const OceaCrustTimeStepID id, double & Thikness) = 0;

      /// @brief Set oceanic crust thickness value in oceanic crustal thickness Io table for
      /// @param[in] id time step ID
      /// @param[in] value specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setOceaCrustThickness(const OceaCrustTimeStepID id, const double Thikness) = 0;

      /// @brief Get list of time steps used in the GridMapIoTbl
      /// @return array with IDs of different time steps used in the GridMapIoTbl
      virtual std::vector<GridMapTimeStepID> getGridMapTimeStepsID() const = 0;

      /// @brief Get "ReferredBy" field from GridMapIoTbl for
      /// @param[in] id time step ID
      /// @param[out] table name specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getReferredBy(const GridMapTimeStepID id, std::string & mapName) = 0;

      /// @brief Set ReferredBy field in GridMapIoTbl for
      /// @param[in] id time step ID
      /// @param[in] table name specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setReferredBy(const GridMapTimeStepID id, const std::string & mapName) = 0;

      /// @}
   protected:
      /// @{
      /// Constructors/destructor

      /// @brief Constructor which creates an empty model
      BottomBoundaryManager() = default;

      /// @brief Destructor, no any actual work is needed here, all is done in the implementation part
      virtual ~BottomBoundaryManager() = default;

      /// @}

   private:
      /// @{
      /// Copy constructor and copy operator are disabled
      BottomBoundaryManager(const BottomBoundaryManager & otherBottomBoundaryManager);
      BottomBoundaryManager & operator = (const BottomBoundaryManager & otherBottomBoundaryManager);
      /// @}
   };
}

#endif // CMB_BOTTOM_BOUNDARY_MANAGER_API

