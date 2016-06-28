//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CRUSTALTHICKNESS_TTSCALCULATOR_H_
#define _CRUSTALTHICKNESS_TTSCALCULATOR_H_

#include "AbstractInterfaceOutput.h"
#include "InterfaceDefs.h"
#include "AbstractValidator.h"

// CBMGenerics library
#include "Polyfunction.h"

// Geophysics library
#include "Local2DArray.h"

//DataAccess library
#include "Interface/GridMap.h"

using namespace DataAccess;

/// @class TotalTectonicSubsidenceCalculator The TTS calculator
class TotalTectonicSubsidenceCalculator {

   typedef GeoPhysics::Local2DArray <CBMGenerics::Polyfunction> PolyFunction2DArray;

   public:
   
      /// @brief Constructs the TTS calculator in order to compute the total tectonic subsidence
      TotalTectonicSubsidenceCalculator( const unsigned int firstI,
                                         const unsigned int firstJ,
                                         const unsigned int lastI,
                                         const unsigned int lastJ,
                                         const double age,
                                         const double airCorrection,
                                         const Interface::GridMap* previousTTS,
                                         const PolyFunction2DArray& depthWaterBottom,
                                         AbstractInterfaceOutput& outputData,
                                         AbstractValidator&       validator );

      ~TotalTectonicSubsidenceCalculator() {};

      /// @brief Computes the total tectonic subsidence map
      void compute();

      /// @return The total tectonic subsidence
      double calculateTTS( const double waterBottom,
                           const double backstrip ) const;

      /// @return The incremental total tectonic subsidence
      double calculateIncrementalTTS( const double TTS,
                                      const double previousTTS ) const;

   private:

      const unsigned int m_firstI; ///< First i index on the map
      const unsigned int m_firstJ; ///< First j index on the map
      const unsigned int m_lastI;  ///< Last i index on the map
      const unsigned int m_lastJ;  ///< Last j index on the map

      const double m_age;           ///< Age of the snapshot at which the TTS is computed
      const double m_airCorrection; ///< The backstrip air correction to be used when the water bottom is above the see level 0m

      const Interface::GridMap* m_previousTTS; ///< The TTS at the previous time step (in descending order xxma-->0Ma)

      const PolyFunction2DArray& m_surfaceDepthHistory; ///< The user defined paleobathymetrie (loaded from the project handle)

      AbstractInterfaceOutput& m_outputData; ///< The global interface output object (contains the output maps)
      AbstractValidator&       m_validator;  ///< The validator to check if a node (i,j) is valid or not
};
#endif

