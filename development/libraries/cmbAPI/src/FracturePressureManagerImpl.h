//                                                                      
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file FracturePressureManager.h
/// @brief This file keeps API declaration which deal with fracture pressure related inputs

#ifndef CMB_FRACTURE_PRESSURE_MANAGER_IMPL_API
#define CMB_FRACTURE_PRESSURE_MANAGER_IMPL_API

#include <memory>

#include "ProjectFileHandler.h"

#include "FracturePressureManager.h"

namespace mbapi {

   // Class FracturePressureManager keeps fracture pressure inputs in Cauldron model and allows to add / delete / edit those inputs
   
   class FracturePressureManagerImpl : public FracturePressureManager
   {
   public:
      // Constructors/destructor
      // brief Constructor which creates an FracturePressureManager
      FracturePressureManagerImpl();

      // Destructor
      virtual ~FracturePressureManagerImpl() { ; }

      // Copy operator
      FracturePressureManagerImpl & operator = (const FracturePressureManagerImpl & otherFluidMgr);

      // Set project database. Reset all
      void setDatabase(database::ProjectFileHandlerPtr pfh);

      /// @{
      /// @brief Get Fracture Pressure model name specified in the RunOptionsIoTbl
      virtual ReturnCode getFracturePressureFunctionName(std::string & FPModelName);

      /// @{
      /// @brief Set Fracture Pressure model name in the RunOptionsIoTbl
      virtual ReturnCode setFracturePressureFunctionName(const std::string & FPModelName);

      /// @brief Get list of models specified in the PressureFuncIo table
      /// @return array with IDs of different models used in the PressureFuncIo table
      virtual std::vector<pressFuncTblID> getpressFuncTblID() const;

      /// @brief Get pressure model name from the PressureFuncIoTbl
      /// @param[in] id fracture pressure model ID
      /// @param[out] Name of the pressure model
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getPresFuncName(const pressFuncTblID id, std::string & Name);

      /// @{
      /// @brief Get Fracture Pressure type specified in the RunOptionsIoTbl
      virtual ReturnCode getFracturePressureType(std::string & FPType);

      /// @{
      /// @brief Set Fracture Pressure type in the RunOptionsIoTbl
      virtual ReturnCode setFracturePressureType(const std::string & FPType);

      /// @{
      /// @brief Get Fracture Pressure constraint method specified in the RunOptionsIoTbl
      virtual ReturnCode getFractureConstraintMethod(int & FPMethod);
      /// @{
      /// @brief Set Fracture Pressure constraint method in the RunOptionsIoTbl
      virtual ReturnCode setFractureConstraintMethod(const int & FPMethod);

      /// @brief Get user defined flag from the PressureFuncIoTbl
      /// @param[in] id fracture pressure model ID
      /// @param[out] UserDefinedFlag value of the pressure model
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getUserDefinedFlag(const pressFuncTblID id, int & UserDefinedFlag);

      /// @brief Set user defined flag in the PressureFuncIoTbl
      /// @param[in] id fracture pressure model ID
      /// @param[in] UserDefinedFlag specified for the ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setUserDefinedFlag(const pressFuncTblID id, const int & UserDefinedFlag);


   private:
      // Copy constructor is disabled, use the copy operator instead
      FracturePressureManagerImpl(const FracturePressureManager &);

      static const char * s_runOptionsTableName;
      static const char * s_pressFuncTableName;
      
      database::ProjectFileHandlerPtr m_db;                // cauldron project database
      database::Table               * m_runOptionsIoTbl;   // RunOptionsIo table
      database::Table               * m_pressFuncIoTbl;   // PressureFuncIo table
      
      static const char * s_FracturePressureFunctionFieldName;  // column name for fracture pressure function name field of RunOptionsIoTbl
      static const char * s_FracturePressureTypeFieldName;  // column name for fracture pressure type field of RunOptionsIoTbl
      static const char * s_FracturePressureConstraintFieldName;  // column name for fracture pressure constraint method field of RunOptionsIoTbl
      static const char * s_PressureFunctionFieldName;  // column name for fracture pressure function name field of PressureFuncIoTbl
      static const char * s_UserDefinedFlagFieldName;  // column name for user defined field of PressureFuncIoTbl
      

   };
}

#endif // CMB_FRACTURE_PRESSURE_MANAGER_IMPL_API


