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
/// @brief This file keeps API declaration for manipulating CTC inputs in Cauldron model

#ifndef CMB_CTC_MANAGER_IMPL_API
#define CMB_CTC_MANAGER_IMPL_API

#include <memory>

#include "ProjectFileHandler.h"

#include "CtcManager.h"

namespace mbapi {

   // Class CtcManager keeps a list of inputs for CTC in Cauldron model and allows to add/delete/edit those
   class CtcManagerImpl : public CtcManager
   {
   public:
      // Constructors/destructor
      // brief Constructor which creates an CtcManager
      CtcManagerImpl();

      // Destructor
      virtual ~CtcManagerImpl() { ; }

      // Copy operator
      CtcManagerImpl & operator = (const CtcManagerImpl & otherFluidMgr);

     

      // Set project database. Reset all
      void setDatabase(database::ProjectFileHandlerPtr pfh);

      /// @{
      /// @brief Get filter half width value
      virtual ReturnCode getFilterHalfWidthValue(int & FiltrHalfWidth);

      /// @{
      /// @brief Set filter half width value
      virtual ReturnCode setFilterHalfWidthValue(const int & FiltrHalfWidth);

      /// @{
      /// @brief Get Upper Lower Continental Crust Ratio value
      virtual ReturnCode getUpperLowerContinentalCrustRatio(double & ULContCrustRatio);
      
      /// @{
      /// @brief Set UpperLowerContinentalCrustRatio value
      virtual ReturnCode setUpperLowerContinentalCrustRatio(const double & ULContCrustRatio);

      /// @{
      /// @brief Get UpperLowerOceanicCrustRatio value
      virtual ReturnCode getUpperLowerOceanicCrustRatio(double & ULOceanicCrustRatio);

      /// @{
      /// @brief Set UpperLowerOceanicCrustRatio value
      virtual ReturnCode setUpperLowerOceanicCrustRatio(const double & ULOceanicCrustRatio);

      /// @brief Get list of stratgraphic layers used in the StratIo table
      /// @return array with IDs of different stratgraphic layers used in the StratIo table
      virtual std::vector<StratigraphyTblLayerID> getStratigraphyTblLayerID() const;

      /// @brief Get depositional age from the StartIoTbl
      /// @param[in] id Stratigraphic layer ID
      /// @param[out] depositional age of the specified stratigraphic layer ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getDepoAge(const StratigraphyTblLayerID id, double & DepoAge);

      /// @brief Get list of time steps defined in CTCRiftingHistoryIoTbl
      /// @return array with IDs of time steps 
      virtual std::vector<TimeStepID> getTimeStepID() const;

      /// @brief Get age from the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[out] age specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getCTCRiftingHistoryTblAge(const TimeStepID id, double & Age);

      /// @brief Set age in the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[in] age specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setCTCRiftingHistoryTblAge(const TimeStepID id, const double Age);

      /// @{
      /// @brief Get Rifting end age grid map
      virtual ReturnCode getEndRiftingAgeMap(std::string & TRIniMapName) ;

	  /// @{
	  /// @brief Set Rifting end age grid map
	  virtual ReturnCode setEndRiftingAgeMap(const std::string & TRIniMapName);
	  
      /// @{
      /// @brief Get Rifting end age scalar value
      virtual ReturnCode getEndRiftingAge(double & RiftingEndAge);

      /// @brief Get tectonic context from the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[out] tectonicContext specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getTectonicFlag(const TimeStepID id, std::string & tectonicContext);

      /// @brief Set tectonic context in the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[in] tectonicContext specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setTectonicFlag(const TimeStepID id, const std::string & tectonicContext);

      /// @{
      /// @brief Get Residual Depth Anomaly value
      virtual ReturnCode getResidualDepthAnomalyScalar(double & RDAvalue);

      /// @{
      /// @brief Set Residual Depth Anomaly value
      virtual ReturnCode setResidualDepthAnomalyScalar(const double & RDAvalue);

      /// @{
      /// @brief Get Residual Depth Anomaly map
      virtual ReturnCode getResidualDepthAnomalyMap(std::string & RDAmap);

      /// @{
      /// @brief Set Residual Depth Anomaly map
      virtual ReturnCode setResidualDepthAnomalyMap(const std::string & RDAmap);

      /// @brief Get Relative sealevel adjustment from the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[out] depth specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getRiftingTblResidualDepthAnomalyScalar(const TimeStepID id, double & depth);

      /// @brief Set Relative sealevel adjustment in the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[in] depth specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setRiftingTblResidualDepthAnomalyScalar(const TimeStepID id, const double & depth);

      /// @brief Get Relative sealevel adjustment map from the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[out] depthMap specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getRiftingTblResidualDepthAnomalyMap(const TimeStepID id, std::string & depthMap);

      /// @brief Set Relative sealevel adjustment map in the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[in] depthMap specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setRiftingTblResidualDepthAnomalyMap(const TimeStepID id, const std::string & depthMap);

      /// @{
      /// @brief Get Maximum thickness of basalt melt value
      virtual ReturnCode getBasaltMeltThicknessValue(double & basaltThicknessvalue);

      /// @{
      /// @brief Set Maximum thickness of basalt melt value
      virtual ReturnCode setBasaltMeltThicknessValue(const double & basaltThicknessvalue);

      /// @{
      /// @brief Get Maximum thickness of basalt melt map
      virtual ReturnCode getBasaltMeltThicknessMap(std::string & basaltThicknessMap);

      /// @{
      /// @brief Set Maximum thickness of basalt melt map
      virtual ReturnCode setBasaltMeltThicknessMap(const std::string & basaltThicknessvMap);

      /// @brief Get maximum thickness of basalt melt from the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[out] DepthValue specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getRiftingTblBasaltMeltThicknessScalar(const TimeStepID id, double & DepthValue);

      /// @brief Set maximum thickness of basalt melt in the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[in] DepthValue specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setRiftingTblBasaltMeltThicknessScalar(const TimeStepID id, const double & DepthValue);

      /// @brief Get maximum thickness of basalt melt map from the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[out] DepthMap specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getRiftingTblBasaltMeltThicknessMap(const TimeStepID id, std::string & DepthMap);

      /// @brief Set maximum thickness of basalt melt map in the CTCRiftingHistoryIoTbl
      /// @param[in] id time step ID
      /// @param[in] DepthMap specified for the time step ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setRiftingTblBasaltMeltThicknessMap(const TimeStepID id, const std::string & DepthMap);

      /// @brief Get list of rows defined in GridMapIoTbl
      /// @return array with IDs of rows 
      virtual std::vector<GridMapID> getGridMapID() const;

      /// @brief Get the table name referred in the GridMapIoTbl
      /// @param[in] id grid map ID
      /// @param[out] tableName specified for the grid map ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getGridMapTablename(const GridMapID id, std::string & tableName);

      /// @brief Set the table name referred in the GridMapIoTbl
      /// @param[in] id grid map ID
      /// @param[in] tableName specified for the grid map ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setGridMapTablename(const GridMapID id, const std::string & tableName);

      /// @brief Get the map name referred in the GridMapIoTbl
      /// @param[in] id grid map ID
      /// @param[out] tableName specified for the grid map ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getGridMapIoTblMapName(const GridMapID id, std::string & tableName);

      /// @brief Set the map name referred in the GridMapIoTbl
      /// @param[in] id grid map ID
      /// @param[in] tableName specified for the grid map ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setGridMapIoTblMapName(const GridMapID id, const std::string & tableName);

   private:
      // Copy constructor is disabled, use the copy operator instead
      CtcManagerImpl(const CtcManager &);

      static const char * s_ctcTableName; 
      static const char * s_stratIoTableName;
      static const char * s_ctcRiftingHistoryIoTableName;
      static const char * s_gridMapIoTableName;

      database::ProjectFileHandlerPtr m_db;                // cauldron project database
      database::Table               * m_ctcIoTbl;          // CTC Io table
      database::Table               * m_stratIoTbl;          // StratIo table
      database::Table               * m_ctcRifthingHistoryIoTbl; // CTCRiftingHistoryIoTbl
      database::Table               * m_gridMapIoTbl; // GridMapIoTbl
      
      static const char * s_FilterHalfWidthFieldName;  // column name for filter half width field
      static const char * s_ULContCrustRatioFieldName;  // column name for filter half width field             
      static const char * s_ULOceaCrustRatioFieldName;  // column name for filter half width field 
      static const char * s_FormationDepoAgeFieldName;  // column name for depositional age of a formation in StratIoTbl
      static const char * s_ctcRiftingHistoryAgeFieldName;  // column name for age of in CTCRiftingHistoryIoTbl
      static const char * s_endRiftingAgeGridFieldName;  // column name for end rifting age map
      static const char * s_endRiftingAgeFieldName;  // column name for end rifting age scalar value
      static const char * s_tectonicContextFieldName;  // column name for tectonicFlag
      static const char * s_rdaScalarFieldName;  // column name for RDA scalar
      static const char * s_rdaMapFieldName;  // column name for RDA map
      static const char * s_basaltThicknessScalarFieldName;  // column name for maximum thickness of basalt melt
      static const char * s_basaltThicknessMapFieldName;  // column name for maximum thickness of basalt melt map
      static const char * s_gridMapReferredByFieldName;  // column name for referring the IoTbl name in the GridMapIoTbl
      static const char * s_gridMapIoTblMapNameFieldName;  // column name for referred map name in the GridMapIoTbl

   };
}

#endif // CMB_CTC_MANAGER_IMPL_API

