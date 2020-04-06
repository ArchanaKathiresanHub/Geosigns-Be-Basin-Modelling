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
/// @brief This file keeps API declaration for manipulating CTCv1 inputs as required in CTCv2 in Cauldron model

#ifndef CMB_CTC_MANAGER_API
#define CMB_CTC_MANAGER_API

#include <vector>
#include "ErrorHandler.h"

/// @page CtcManagerPage CTC Manager
/// @link mbapi::CtcManager CTC manager @endlink provides set of interfaces to create/delete/edit list of CTC inputs
/// in the data model. Also it has a set of interfaces to get/set property of any CTC inputs from CTC related Io tables
///

namespace mbapi {
   /// @class CtcManager CtcManager.h "CtcManager.h" 
   /// @brief Class CtcManager keeps a list of CTC inputs in Cauldron model and allows to add/delete/edit those inputs 
   class CtcManager : public ErrorHandler
   {
   public:

      /// @{
      /// Set of interfaces for interacting with a Cauldron model
      typedef size_t StratigraphyTblLayerID;  ///< unique ID for stratigraphic layers defined in the StratIo table
      typedef size_t TimeStepID;  ///< unique ID for time steps defined in the CTCRiftingHistoryIo table
      typedef size_t GridMapID;  ///< unique ID for time steps defined in the GridMapIo table

      /// @{
      /// @brief Get FilterHalfWidth value
      virtual ReturnCode getFilterHalfWidthValue(int & FiltrHalfWidth) = 0;

      /// @{
      /// @brief Set FilterHalfWidth value
      virtual ReturnCode setFilterHalfWidthValue(const int & FiltrHalfWidth) = 0;

      /// @{
      /// @brief Get UpperLowerContinentalCrustRatio value
      virtual ReturnCode getUpperLowerContinentalCrustRatio(double & ULContCrustRatio) = 0;

      /// @{
      /// @brief Set UpperLowerContinentalCrustRatio value
      virtual ReturnCode setUpperLowerContinentalCrustRatio(const double & ULContCrustRatio) = 0;

      /// @{
      /// @brief Get UpperLowerOceanicCrustRatio value
      virtual ReturnCode getUpperLowerOceanicCrustRatio(double & ULOceanicCrustRatio) = 0;

      /// @{
      /// @brief Set UpperLowerOceanicCrustRatio value
      virtual ReturnCode setUpperLowerOceanicCrustRatio(const double & ULOceanicCrustRatio) = 0;

      /// @brief Get list of stratgraphic layers used in the StratIo table
      /// @return array with IDs of different stratgraphic layers used in the StratIo table
      virtual std::vector<StratigraphyTblLayerID> getStratigraphyTblLayerID() const = 0;

      /// @brief Get depositional age from the StartIoTbl
      /// @param[in] id Stratigraphic layer ID
      /// @param[out] depositional age of the specified stratigraphic layer ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getDepoAge(const StratigraphyTblLayerID id, double & DepoAge) = 0;

      /// @brief Get list of time steps defined in CTCRiftingHistoryIoTbl
      /// @return array with IDs of time steps 
      virtual std::vector<TimeStepID> getTimeStepID() const = 0;

      /// @brief Get age from the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[out] age specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getCTCRiftingHistoryTblAge(const TimeStepID id, double & Age) = 0;

      /// @brief Set age in the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[in] age specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setCTCRiftingHistoryTblAge(const TimeStepID id, const double Age) = 0;

      /// @{
      /// @brief Get Rifting end age grid map
      virtual ReturnCode getEndRiftingAgeMap(std::string & TRIniMapName) = 0;

	  /// @{
	  /// @brief Set Rifting end age grid map
	  virtual ReturnCode setEndRiftingAgeMap(const std::string & TRIniMapName) = 0;
	  
      /// @{
      /// @brief Get Rifting end age scalar value
      virtual ReturnCode getEndRiftingAge(double & RiftingEndAge) = 0;

      /// @brief Get tectonic context from the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[out] tectonicContext specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getTectonicFlag(const TimeStepID id, std::string & tectonicContext) = 0;

      /// @brief Set tectonic context in the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[in] tectonicContext specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setTectonicFlag(const TimeStepID id, const std::string & tectonicContext) = 0;

      /// @{
      /// @brief Get Residual Depth Anomaly value
      virtual ReturnCode getResidualDepthAnomalyScalar(double & RDAvalue) = 0;

      /// @{
      /// @brief Set Residual Depth Anomaly value
      virtual ReturnCode setResidualDepthAnomalyScalar(const double & RDAvalue) = 0;

      /// @{
      /// @brief Get Residual Depth Anomaly map
      virtual ReturnCode getResidualDepthAnomalyMap(std::string & RDAmap) = 0;

      /// @{
      /// @brief Set Residual Depth Anomaly map
      virtual ReturnCode setResidualDepthAnomalyMap(const std::string & RDAmap) = 0;

      /// @brief Get Relative sealevel adjustment from the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[out] depth specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getRiftingTblResidualDepthAnomalyScalar(const TimeStepID id, double & depth) = 0;

      /// @brief Set Relative sealevel adjustment in the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[in] depth specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setRiftingTblResidualDepthAnomalyScalar(const TimeStepID id, const double & depth) = 0;

      /// @brief Get Relative sealevel adjustment map from the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[out] depthMap specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getRiftingTblResidualDepthAnomalyMap(const TimeStepID id, std::string & depthMap) = 0;

      /// @brief Set Relative sealevel adjustment map in the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[in] depthMap specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setRiftingTblResidualDepthAnomalyMap(const TimeStepID id, const std::string & depthMap) = 0;

      /// @{
      /// @brief Get Maximum thickness of basalt melt value from the CTCIoTbl
      virtual ReturnCode getBasaltMeltThicknessValue(double & basaltThicknessvalue) = 0;

      /// @{
      /// @brief Set Maximum thickness of basalt melt value from the CTCIoTbl
      virtual ReturnCode setBasaltMeltThicknessValue(const double & basaltThicknessvalue) = 0;

      /// @{
      /// @brief Get Maximum thickness of basalt melt map from the CTCIoTbl
      virtual ReturnCode getBasaltMeltThicknessMap(std::string & basaltThicknessMap) = 0;

      /// @{
      /// @brief Set Maximum thickness of basalt melt map from the CTCIoTbl
      virtual ReturnCode setBasaltMeltThicknessMap(const std::string & basaltThicknessvMap) = 0;

      /// @brief Get maximum thickness of basalt melt from the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[out] DepthValue specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getRiftingTblBasaltMeltThicknessScalar(const TimeStepID id, double & DepthValue) = 0;

      /// @brief Set maximum thickness of basalt melt in the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[in] DepthValue specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setRiftingTblBasaltMeltThicknessScalar(const TimeStepID id, const double & DepthValue) = 0;

      /// @brief Get maximum thickness of basalt melt map from the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[out] DepthMap specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getRiftingTblBasaltMeltThicknessMap(const TimeStepID id, std::string & DepthMap) = 0;

      /// @brief Set maximum thickness of basalt melt map in the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[in] DepthMap specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setRiftingTblBasaltMeltThicknessMap(const TimeStepID id, const std::string & DepthMap) = 0;

      /// @brief Get list of rows defined in GridMapIoTbl
      /// @return array with IDs of rows 
      virtual std::vector<GridMapID> getGridMapID() const = 0;

      /// @brief Get the table name referred in the GridMapIoTbl
      /// @param[in] id grid map ID
      /// @param[out] tableName specified for the grid map ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getGridMapTablename(const GridMapID id, std::string & tableName) = 0;

      /// @brief Set the table name referred in the GridMapIoTbl
      /// @param[in] id grid map ID
      /// @param[in] tableName specified for the grid map ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setGridMapTablename(const GridMapID id, const std::string & tableName) = 0;

      /// @brief Get the map name referred in the GridMapIoTbl
      /// @param[in] id grid map ID
      /// @param[out] tableName specified for the grid map ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getGridMapIoTblMapName(const GridMapID id, std::string & tableName) = 0;

      /// @brief Set the map name referred in the GridMapIoTbl
      /// @param[in] id grid map ID
      /// @param[in] tableName specified for the grid map ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setGridMapIoTblMapName(const GridMapID id, const std::string & tableName) = 0;

      /// @}
   protected:
      /// @{
      /// Constructors/destructor

      /// @brief Constructor which creates an empty model
      CtcManager() = default;

      /// @brief Destructor, no any actual work is needed here, all is done in the implementation part
      virtual ~CtcManager() = default;

      /// @}

   private:
      /// @{
      /// Copy constructor and copy operator are disabled
      CtcManager(const CtcManager & otherCtcManager);
      CtcManager & operator = (const CtcManager & otherCtcManager);
      /// @}
   };
}

#endif // CMB_CTC_MANAGER_API


