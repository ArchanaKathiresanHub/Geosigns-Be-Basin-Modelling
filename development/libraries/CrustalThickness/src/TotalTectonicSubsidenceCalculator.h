//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CRUSTALTHICKNESS_TTSCALCULATOR_H_
#define _CRUSTALTHICKNESS_TTSCALCULATOR_H_

// CBMGenerics library
#include "Polyfunction.h"

// DataAcccess library
#include "Local2DArray.h"

// forward declarations
namespace DataAccess
{
   namespace Interface
   {
      class GridMap;
   }
}
class InterfaceInput;
namespace DataModel {
   class AbstractValidator;
}
class AbstractInterfaceOutput;

using namespace DataAccess;

namespace  CrustalThickness
{
   /// @class TotalTectonicSubsidenceCalculator The TTS calculator
   class TotalTectonicSubsidenceCalculator {

      typedef Interface::Local2DArray <CBMGenerics::Polyfunction> PolyFunction2DArray;

   public:

      /// @brief Constructs the TTS calculator in order to compute the total tectonic subsidence
      TotalTectonicSubsidenceCalculator( InterfaceInput& inputData,
         AbstractInterfaceOutput&      outputData,
         const DataModel::AbstractValidator& validator,
         const double                  age,
         const double                  airCorrection,
         const Interface::GridMap*     previousTTS,
         const PolyFunction2DArray&    depthWaterBottom );

      ~TotalTectonicSubsidenceCalculator() = default;

      /// @brief Computes the total tectonic subsidence map
      void compute();

      /// @return The total tectonic subsidence
      double calculateTTS( const double waterBottom,
         const double backstrip ) const;

      /// @return The incremental total tectonic subsidence
      double calculateIncrementalTTS( const double TTS,
         const double previousTTS ) const;

      /// @return The sea level adjusted total or incremental tectonic subsidence
      double calculateTSadjusted( const double TTS,
         const double seaLevelAdjustment ) const;

   private:

      /// @defgroup DataUtilities
      /// @{
      /// @brief Retrieve basement depth data
      void retrieveData();
      /// @brief Restore basement depth data
      void restoreData();
      /// @}

      const unsigned int m_firstI; ///< First i index on the map
      const unsigned int m_firstJ; ///< First j index on the map
      const unsigned int m_lastI;  ///< Last i index on the map
      const unsigned int m_lastJ;  ///< Last j index on the map

      const double m_age;           ///< Age of the snapshot at which the TTS is computed
      const double m_airCorrection; ///< The backstrip air correction to be used when the water bottom is above the see level 0m

      const Interface::GridMap* m_previousTTS;          ///< The TTS at the previous time step (in descending order xxma-->0Ma) [m]
      const Interface::GridMap& m_seaLevelAdjustment;   ///< The sea level adjustment                                           [m]
      const PolyFunction2DArray& m_surfaceDepthHistory; ///< The user defined paleobathymetrie (loaded from the project handle) [m]

      AbstractInterfaceOutput&      m_outputData;      ///< The global interface output object (contains the output maps)
      const DataModel::AbstractValidator& m_validator; ///< The validator to check if a node (i,j) is valid or not
   };
} // End namespace  CrustalThickness
#endif

