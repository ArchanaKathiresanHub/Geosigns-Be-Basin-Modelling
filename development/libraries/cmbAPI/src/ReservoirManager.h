//                                                                      
// Copyright (C) 2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell .
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ReservoirManager.h
/// @brief This file keeps API declaration for manipulating Reservoirs in Cauldron model

#ifndef CMB_RESERVOIR_MANAGER_API
#define CMB_RESERVOIR_MANAGER_API

#include <vector>
#include "ErrorHandler.h"

/// @page ReservoirManagerPage Reservoir Manager
/// @link mbapi::ReservoirManager Reservoir manager @endlink provides set of interfaces to create/delete/edit list Reservoirs
/// in the data model. Also it has set of interfaces to get/set property of any Reservoir from the list
///

namespace mbapi {
   /// @class ReservoirManager ReservoirManager.h "ReservoirManager.h" 
   /// @brief Class ReservoirManager keeps a list of Reservoirs in Cauldron model and allows to add/delete/edit Reservoir
   class ReservoirManager : public ErrorHandler
   {
   public:

      /// @{
      /// Set of interfaces for interacting with a Cauldron model
      typedef size_t ReservoirID;  ///< unique ID for Reservoir

                                   /// @brief Get list of Reservoirs in the model
                                   /// @return array with IDs of different reservoirs defined in the model
      virtual std::vector<ReservoirID> getReservoirsID() const = 0;
      virtual ReturnCode getResName(const ReservoirID id, std::string & fluidName) = 0;

      /// @brief get reservoir trapcapacity value for the given reservoir ID
      /// @param[in] id Reservoir ID
      /// @param[out] trapCapacity value of trap capacity for the given Reservoir
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode getResCapacity(const ReservoirID id, double & trapCapacity) = 0;
      /// @brief set globle reservoir trapcapacity value for the given reservoir ID in ReservoirIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] trapCapacity globle trap capacity value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResCapacity(const ReservoirID id, const double & trapCapacity) = 0;
      /// @brief set globle reservoir trapcapacity value for the given reservoir ID in ReservoirOptionsIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] trapCapacity globle trap capacity value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOptionsCapacity(const ReservoirID id, const double & trapCapacity) = 0;

      /// @brief get reservoir bioDegradInd value for the given reservoir ID
      /// @param[in] id Reservoir ID
      /// @param[out] bioDegradInd value for the given Reservoir
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode getResBioDegradInd(const ReservoirID id, int & bioDegradInd) = 0;
      /// @brief set globle reservoir bioDegradInd value for the given reservoir ID in ReservoirIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] bioDegradInd globle bioDegradInd value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResBioDegradInd(const ReservoirID id, const int & bioDegradInd) = 0;
      /// @brief set globle reservoir bioDegradInd value for the given reservoir ID in ReservoirOptionsIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] bioDegradInd globle bioDegradInd value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOptionsBioDegradInd(const ReservoirID id, const int & bioDegradInd) = 0;

      /// @brief get reservoir oilToGasCrackingInd value for the given reservoir ID
      /// @param[in] id Reservoir ID
      /// @param[out] oilToGasCrackingInd value of oilToGasCrackingInd for the given Reservoir
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode getResOilToGasCrackingInd(const ReservoirID id, int & oilToGasCrackingInd) = 0;
      /// @brief set globle reservoir oilToGasCrackingInd value for the given reservoir ID in ReservoirIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] oilToGasCrackingInd globle oilToGasCrackingInd value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOilToGasCrackingInd(const ReservoirID id, const int & oilToGasCrackingInd) = 0;
      /// @brief set globle reservoir oilToGasCrackingInd value for the given reservoir ID in ReservoirOptionsIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] oilToGasCrackingInd globle oilToGasCrackingInd value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOptionsOilToGasCrackingInd(const ReservoirID id, const int & oilToGasCrackingInd) = 0;
      
      /// @brief get reservoir blockingInd value for the given reservoir ID
      /// @param[in] id Reservoir ID
      /// @param[out] blockingInd value of blockingInd for the given Reservoir
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode getResBlockingInd(const ReservoirID id, int & blockingInd) = 0;
      /// @brief set globle reservoir blockingInd value for the given reservoir ID in ReservoirIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] blockingInd globle blockingInd value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResBlockingInd(const ReservoirID id, const int & blockingInd) = 0;
      /// @brief set globle reservoir blockingInd value for the given reservoir ID in ReservoirOptionsIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] blockingInd globle blockingInd value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOptionsBlockingInd(const ReservoirID id, const int & blockingInd) = 0;

      /// @brief get reservoir blockingPermeability value for the given reservoir ID
      /// @param[in] id Reservoir ID
      /// @param[out] blockingPermeability value of blockingPermeability for the given Reservoir
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode getResBlockingPermeability(const ReservoirID id, double & blockingPermeability) = 0;
      /// @brief set globle reservoir blockingPermeability value for the given reservoir ID in ReservoirIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] blockingPermeability globle blockingPermeability value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResBlockingPermeability(const ReservoirID id, const double & blockingPermeability) = 0;
      /// @brief set globle reservoir blockingPermeability value for the given reservoir ID in ReservoirOptionsIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] blockingPermeability globle blockingPermeability value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOptionsBlockingPermeability(const ReservoirID id, const double & blockingPermeability) = 0;

      /// @brief set globle reservoir diffusionInd value for the given reservoir ID in ReservoirIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] diffusionInd globle diffusionInd value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResDiffusionInd(const ReservoirID id, const int & diffusionInd) = 0;
      /// @brief set globle reservoir diffusionInd value for the given reservoir ID in ReservoirOptionsIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] diffusionInd globle diffusionInd value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOptionsDiffusionInd(const ReservoirID id, const int & diffusionInd) = 0;

      /// @brief set globle reservoir minOilColumnHeight value for the given reservoir ID in ReservoirIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] minOilColumnHeight globle minOilColumnHeight value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResMinOilColumnHeight(const ReservoirID id, const double & minOilColumnHeight) = 0;
      /// @brief set globle reservoir minOilColumnHeight value for the given reservoir ID in ReservoirOptionsIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] minOilColumnHeight globle minOilColumnHeight value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOptionsMinOilColumnHeight(const ReservoirID id, const double & minOilColumnHeight) = 0;

      /// @brief set globle reservoir minGasColumnHeight value for the given reservoir ID in ReservoirIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] minGasColumnHeight globle minGasColumnHeight value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResMinGasColumnHeight(const ReservoirID id, const double & minGasColumnHeight) = 0;
      /// @brief set globle reservoir minGasColumnHeight value for the given reservoir ID in ReservoirOptionsIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] minGasColumnHeight globle minGasColumnHeight value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOptionsMinGasColumnHeight(const ReservoirID id, const double & minGasColumnHeight) = 0;

      /// @brief set globle reservoir blockingPorosity value for the given reservoir ID in ReservoirIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] blockingPorosity globle blockingPorosity value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResBlockingPorosity(const ReservoirID id, const double & blockingPorosity) = 0;
      /// @brief set globle reservoir blockingPorosity value for the given reservoir ID in ReservoirOptionsIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] blockingPorosity globle blockingPorosity value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOptionsBlockingPorosity(const ReservoirID id, const double & blockingPorosity) = 0;

	  /// @brief get activity mode for the given reservoir ID
	  /// @param[in] id Reservoir ID
	  /// @param[out] activitymode for the given Reservoir
	  /// @return NoError on success or OutOfRangeValue or NonexistingID on error
	  virtual ReturnCode getResActivityMode(const ReservoirID id, std::string & activitymode) = 0;
	  /// @brief set globle reservoir blockingPermeability value for the given reservoir ID in ReservoirIoTbl
	  /// @param[in] id Reservoir ID
	  /// @param[in] activitymode activity mode for the given Reservoir
	  /// @return NoError on success or OutOfRangeValue or NonexistingID on error
	  virtual ReturnCode setResActivityMode(const ReservoirID id, const std::string & activitymode) = 0;

	  /// @brief get reservoir activity start age value for the given reservoir ID in ReservoirIoTbl
	  /// @param[in] id Reservoir ID
	  /// @param[out] age Age from which the reservoir is active for the given reservoir ID
	  /// @return NoError on success or OutOfRangeValue or NonexistingID on error
	  virtual ReturnCode getResActivityStartAge(const ReservoirID id, double & age) = 0;
	  /// @brief set reservoir activity start age value for the given reservoir ID in ReservoirIoTbl
	  /// @param[in] id Reservoir ID
	  /// @param[in] age Age from which the reservoir is active for the given reservoir ID
	  /// @return NoError on success or OutOfRangeValue or NonexistingID on error
	  virtual ReturnCode setResActivityStartAge(const ReservoirID id, const double & blockingPorosity) = 0;

	  /// @brief get minimum oil column height for the given reservoir ID in ReservoirIoTbl
	  /// @param[in] id Reservoir ID
	  /// @param[out] minOilColumnHeight Minimum oil column height specified for the given reservoir ID
	  /// @return NoError on success or OutOfRangeValue or NonexistingID on error
	  virtual ReturnCode getResMinOilColumnHeight(const ReservoirID id, double & minOilColumnHeight) = 0;

	  /// @brief get minimum gas column height for the given reservoir ID in ReservoirIoTbl
	  /// @param[in] id Reservoir ID
	  /// @param[out] minGasColumnHeight Minimum gas column height specified for the given reservoir ID
	  /// @return NoError on success or OutOfRangeValue or NonexistingID on error
	  virtual ReturnCode getResMinGasColumnHeight(const ReservoirID id, double & minGasColumnHeight) = 0;


   protected:
      /// @{
      /// Constructors/destructor

      /// @brief Constructor which creates an empty model
      ReservoirManager() = default;

      /// @brief Destructor, no any actual work is needed here, all is done in the implementation part
      virtual ~ReservoirManager() = default;

      /// @}

   private:
      /// @{
      /// Copy constructor and copy operator are disabled
      ReservoirManager(const ReservoirManager & otherReservoirManager);
      ReservoirManager & operator = (const ReservoirManager & otherReservoirManager);
      /// @}
   };
}

#endif // CMB_RESERVOIR_MANAGER_API


