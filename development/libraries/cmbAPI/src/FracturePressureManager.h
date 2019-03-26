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

#ifndef CMB_FRACTURE_PRESSURE_MANAGER_API
#define CMB_FRACTURE_PRESSURE_MANAGER_API

#include <vector>
#include "ErrorHandler.h"

/// @page FracturePressureManagerPage FracturePressure Manager
/// @link mbapi::FracturePressureManager FracturePressure manager @endlink provides set of interfaces to interfaces to load/modify/save fracture pressure related inputs in project file

namespace mbapi {
   /// @class FracturePressureManager FracturePressureManager.h "FracturePressureManager.h" 
   /// @brief Class FracturePressureManager keeps fracture pressure inputs in Cauldron model and allows to add/delete/edit those inputs 
   class FracturePressureManager : public ErrorHandler
   {
   public:

      /// @{
      /// Set of interfaces for interacting with a Cauldron model
      typedef size_t pressFuncTblID;  ///< unique ID for pressure models defined in the PressureFuncIo table

      /// @{
      /// @brief Get Fracture Pressure model name specified in the RunOptionsIoTbl
      virtual ReturnCode getFracturePressureFunctionName(std::string & FPModelName) = 0;

      /// @{
      /// @brief Set Fracture Pressure model name in the RunOptionsIoTbl
      virtual ReturnCode setFracturePressureFunctionName(const std::string & FPModelName) = 0;

      /// @brief Get list of models specified in the PressureFuncIo table
      /// @return array with IDs of different models used in the PressureFuncIo table
      virtual std::vector<pressFuncTblID> getpressFuncTblID() const = 0;

      /// @brief Get pressure model name from the PressureFuncIoTbl
      /// @param[in] id fracture pressure model ID
      /// @param[out] Name of the pressure model
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getPresFuncName(const pressFuncTblID id, std::string & Name) = 0;

      /// @{
      /// @brief Get Fracture Pressure type specified in the RunOptionsIoTbl
      virtual ReturnCode getFracturePressureType(std::string & FPType) = 0;

      /// @{
      /// @brief Set Fracture Pressure type in the RunOptionsIoTbl
      virtual ReturnCode setFracturePressureType(const std::string & FPType) = 0;

      /// @{
      /// @brief Get Fracture Pressure constraint method specified in the RunOptionsIoTbl
      virtual ReturnCode getFractureConstraintMethod(int & FPMethod) = 0;

      /// @{
      /// @brief Set Fracture Pressure constraint method in the RunOptionsIoTbl
      virtual ReturnCode setFractureConstraintMethod(const int & FPMethod) = 0;

      /// @brief Get user defined flag from the PressureFuncIoTbl
      /// @param[in] id fracture pressure model ID
      /// @param[out] UserDefinedFlag value of the pressure model
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getUserDefinedFlag(const pressFuncTblID id, int & UserDefinedFlag) = 0;

      /// @brief Set user defined flag in the PressureFuncIoTbl
      /// @param[in] id fracture pressure model ID
      /// @param[in] UserDefinedFlag specified for the ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setUserDefinedFlag(const pressFuncTblID id, const int & UserDefinedFlag) = 0;

      /// @}
   protected:
      /// @{
      /// Constructors/destructor

      /// @brief Constructor which creates an empty model
      FracturePressureManager() = default;

      /// @brief Destructor, no any actual work is needed here, all is done in the implementation part
      virtual ~FracturePressureManager() = default;

      /// @}

   private:
      /// @{
      /// Copy constructor and copy operator are disabled
      FracturePressureManager(const FracturePressureManager & otherFracturePressureManager);
      FracturePressureManager & operator = (const FracturePressureManager & otherFracturePressureManager);
      /// @}
   };
}

#endif // CMB_FRACTURE_PRESSURE_MANAGER_API



