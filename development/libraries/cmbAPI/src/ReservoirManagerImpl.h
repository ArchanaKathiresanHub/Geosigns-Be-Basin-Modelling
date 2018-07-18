//                                                                      
// Copyright (C) 2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell .
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ReservoirManagerImpl.h
/// @brief This file keeps API implementation for manipulating Reservoirs in Cauldron model

#ifndef CMB_RESERVOIR_MANAGER_IMPL_API
#define CMB_RESERVOIR_MANAGER_IMPL_API

#include <memory>

#include "ProjectFileHandler.h"

#include "ReservoirManager.h"
#include <stdexcept>

namespace mbapi {

   // Class ReservoirManager keeps a list of Reservoirs in Cauldron model and allows to add/delete/edit Reservoir
   class ReservoirManagerImpl : public ReservoirManager
   {
   public:
      // Constructors/destructor
      // brief Constructor which creates an ReservoirManager
      ReservoirManagerImpl();

      // Destructor
      virtual ~ReservoirManagerImpl() { ; }

      // Copy operator
      ReservoirManagerImpl & operator = (const ReservoirManagerImpl & otherReservoirMgr);

      // Set of interfaces for interacting with a Cauldron model

      // Set project database. Reset all
      void setDatabase(database::ProjectFileHandlerPtr pfh);

      // Get list of Reservoirs in the model
      // return array with IDs of different Reservoirs defined in the model
      virtual std::vector<ReservoirID> getReservoirsID() const;
      virtual ReturnCode getResName(const ReservoirID id, std::string & fluidName);

      /// @brief get reservoir trapcapacity value for the given reservoir ID
      /// @param[in] id Reservoir ID
      /// @param[out] trapCapacity value of trap capacity for the given Reservoir
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode getResCapacity(const ReservoirID id, double & resCap);
      /// @brief set globle reservoir trapcapacity value for the given reservoir ID in ReservoirIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] trapCapacity globle trap capacity value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResCapacity(const ReservoirID id, const double & resCap);
      /// @brief set globle reservoir trapcapacity value for the given reservoir ID in ReservoirOptionsIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] trapCapacity globle trap capacity value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOptionsCapacity(const ReservoirID id, const double & resCap);

      /// @brief get reservoir bioDegradInd value for the given reservoir ID
      /// @param[in] id Reservoir ID
      /// @param[out] bioDegradInd value for the given Reservoir
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode getResBioDegradInd(const ReservoirID id, int & bioDegradInd);
      /// @brief set globle reservoir bioDegradInd value for the given reservoir ID in ReservoirIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] bioDegradInd globle bioDegradInd value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResBioDegradInd(const ReservoirID id, const int & bioDegradInd);
      /// @brief set globle reservoir bioDegradInd value for the given reservoir ID in ReservoirOptionsIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] bioDegradInd globle bioDegradInd value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOptionsBioDegradInd(const ReservoirID id, const int & bioDegradInd);

      /// @brief get reservoir oilToGasCrackingInd value for the given reservoir ID
      /// @param[in] id Reservoir ID
      /// @param[out] oilToGasCrackingInd value of oilToGasCrackingInd for the given Reservoir
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode getResOilToGasCrackingInd(const ReservoirID id, int & oilToGasCrackingInd);
      /// @brief set globle reservoir oilToGasCrackingInd value for the given reservoir ID in ReservoirIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] oilToGasCrackingInd globle oilToGasCrackingInd value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOilToGasCrackingInd(const ReservoirID id, const int & oilToGasCrackingInd);
      /// @brief set globle reservoir oilToGasCrackingInd value for the given reservoir ID in ReservoirOptionsIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] oilToGasCrackingInd globle oilToGasCrackingInd value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOptionsOilToGasCrackingInd(const ReservoirID id, const int & oilToGasCrackingInd);

      /// @brief get reservoir blockingInd value for the given reservoir ID
      /// @param[in] id Reservoir ID
      /// @param[out] blockingInd value of blockingInd for the given Reservoir
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode getResBlockingInd(const ReservoirID id, int & blockingInd);
      /// @brief set globle reservoir blockingInd value for the given reservoir ID in ReservoirIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] blockingInd globle blockingInd value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResBlockingInd(const ReservoirID id, const int & blockingInd);
      /// @brief set globle reservoir blockingInd value for the given reservoir ID in ReservoirOptionsIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] blockingInd globle blockingInd value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOptionsBlockingInd(const ReservoirID id, const int & blockingInd);

      /// @brief get reservoir blockingPermeability value for the given reservoir ID
      /// @param[in] id Reservoir ID
      /// @param[out] blockingPermeability value of blockingPermeability for the given Reservoir
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode getResBlockingPermeability(const ReservoirID id, double & blockingPermeability);
      /// @brief set globle reservoir blockingPermeability value for the given reservoir ID in ReservoirIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] blockingPermeability globle blockingPermeability value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResBlockingPermeability(const ReservoirID id, const double & blockingPermeability);
      /// @brief set globle reservoir blockingPermeability value for the given reservoir ID in ReservoirOptionsIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] blockingPermeability globle blockingPermeability value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOptionsBlockingPermeability(const ReservoirID id, const double & blockingPermeability);

      /// @brief set globle reservoir diffusionInd value for the given reservoir ID in ReservoirIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] diffusionInd globle diffusionInd value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResDiffusionInd(const ReservoirID id, const int & diffusionInd);
      /// @brief set globle reservoir diffusionInd value for the given reservoir ID in ReservoirOptionsIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] diffusionInd globle diffusionInd value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOptionsDiffusionInd(const ReservoirID id, const int & diffusionInd);

      /// @brief set globle reservoir minOilColumnHeight value for the given reservoir ID in ReservoirIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] minOilColumnHeight globle minOilColumnHeight value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResMinOilColumnHeight(const ReservoirID id, const double & minOilColumnHeight);
      /// @brief set globle reservoir minOilColumnHeight value for the given reservoir ID in ReservoirOptionsIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] minOilColumnHeight globle minOilColumnHeight value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOptionsMinOilColumnHeight(const ReservoirID id, const double & minOilColumnHeight);

      /// @brief set globle reservoir minGasColumnHeight value for the given reservoir ID in ReservoirIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] minGasColumnHeight globle minGasColumnHeight value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResMinGasColumnHeight(const ReservoirID id, const double & minGasColumnHeight);
      /// @brief set globle reservoir minGasColumnHeight value for the given reservoir ID in ReservoirOptionsIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] minGasColumnHeight globle minGasColumnHeight value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOptionsMinGasColumnHeight(const ReservoirID id, const double & minGasColumnHeight);

      /// @brief set globle reservoir blockingPorosity value for the given reservoir ID in ReservoirIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] blockingPorosity globle blockingPorosity value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResBlockingPorosity(const ReservoirID id, const double & blockingPorosity);
      /// @brief set globle reservoir blockingPorosity value for the given reservoir ID in ReservoirOptionsIoTbl
      /// @param[in] id Reservoir ID
      /// @param[in] blockingPorosity globle blockingPorosity value
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setResOptionsBlockingPorosity(const ReservoirID id, const double & blockingPorosity);
      


   private:
      // Copy constructor is disabled, use the copy operator instead
      ReservoirManagerImpl(const ReservoirManager &);

      database::ProjectFileHandlerPtr m_db;          // cauldron project database
      database::Table               * m_reservoirIoTbl;  // Reservoir Io table
      database::Table               * m_reservoirOptionsIoTbl;  // Reservoir Options Io table
      static const char * s_reservoirTableName; // name of the reservoir table 
      static const char * s_reservoirOptionsTableName;  // name of the reservoir options table
      static const char * s_trapCapacityFieldName;		// column name for trap capacity
      static const char * s_bioDegradIndFieldName;        // column name for bio-degradation index 
      static const char * s_oilToGasCrackingIndFieldName;        // column name for oil to gas cracking index 
      static const char * s_diffusionIndFieldName;        // column name for diffusion index 
      static const char * s_minOilColumnHeightFieldName;        // column name for oil column height field
      static const char * s_minGasColumnHeightFieldName;   // column name for gas column height field
      static const char * s_blockingIndFieldName;  // column name for blocking index
      static const char * s_blockingPermeabilityFieldName; // column name for blocking permeability field
      static const char * s_blockingPorosityFieldName; // column name for blocking porosity field
      static const char * s_reservoirTypeFieldName;  // name of the reservoir  
   };
}

#endif // CMB_RESERVOIR_MANAGER_IMPL_API

