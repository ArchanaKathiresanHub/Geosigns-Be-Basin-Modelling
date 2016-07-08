//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CRUSTALTHICKNESS_DENSITYCALCULATOR_H_
#define _CRUSTALTHICKNESS_DENSITYCALCULATOR_H_

#include "AbstractInterfaceOutput.h"
#include "AbstractValidator.h"

// DerivedProperties library
#include "SurfaceProperty.h"

// utilitites library
#include "FormattingException.h"

// DataAccess
#include "Interface/CrustalThicknessInterface.h"

using namespace DataAccess;

/// @class DensityCalculator The density calcultor used during the CTC backstriping
class DensityCalculator {

   typedef formattingexception::GeneralException DensityException;

public:

   DensityCalculator( const unsigned int firstI,
                      const unsigned int firstJ,
                      const unsigned int lastI,
                      const unsigned int lastJ,
                      const double theMantleDensity,
                      const double theWaterDensity,
                      DerivedProperties::SurfacePropertyPtr pressureBasement,
                      DerivedProperties::SurfacePropertyPtr pressureWaterBottom,
                      DerivedProperties::SurfacePropertyPtr depthBasement,
                      DerivedProperties::SurfacePropertyPtr depthWaterBottom,
                      AbstractInterfaceOutput& outputData,
                      AbstractValidator&       validator );
   ~DensityCalculator() {};

   /// @brief Computes the semdiments density, the sediments thickness, the backstrip and the compensation maps
   void compute();

   /// @brief Perfrom the backstriping at node (i,j)
   void calculate( const double topBasementDepthValue,
                   const double waterBottomDepthValue,
                   const double pressureTopBasementValue,
                   const double pressureWaterBottomValue );

   /// @defgroup Accessors
   /// @{
   DerivedProperties::SurfacePropertyPtr getDepthBasementMap()       const { return m_pressureBasement; };
   DerivedProperties::SurfacePropertyPtr getDepthWaterBottomMap()    const { return m_pressureWaterBottom; };
   DerivedProperties::SurfacePropertyPtr getPressureBasementMap()    const { return m_depthBasement; };
   DerivedProperties::SurfacePropertyPtr getPressureWaterBottomMap() const { return m_depthWaterBottom; };

   double getBackstrip()             const { return m_backstrip; };
   double getSedimentThickness()     const { return m_sedimentThickness; };
   double getSedimentDensity()       const { return m_sedimentDensity; };
   double getCompensation()          const { return m_compensation; };

   double getDensityTerm()   const { return m_densityTerm; };
   double getAirCorrection() const { return m_airCorrection; };
   /// @}
      

   private:

      /// @brief Compute the density term and the air correction
      void setDensities();

      /// @defgroup DataUtilities
      /// @{
      /// @brief Retrieve pressure and depth maps data
      void retrieveData();
      /// @brief Restore pressure and depth maps data
      void restoreData();
      /// @}

      const unsigned int m_firstI; ///< First i index on the map
      const unsigned int m_firstJ; ///< First j index on the map
      const unsigned int m_lastI;  ///< Last i index on the map
      const unsigned int m_lastJ;  ///< Last j index on the map

      AbstractInterfaceOutput& m_outputData; ///< The global interface output object (contains the output maps)
      AbstractValidator&       m_validator;  ///< The validator to check if a node (i,j) is valid or not
     
      /// @defgroup DerivedProperties
      /// @{
      DerivedProperties::SurfacePropertyPtr m_pressureBasement;    ///< The pressure of the basement at the current snapshot
      DerivedProperties::SurfacePropertyPtr m_pressureWaterBottom; ///< The pressure of the water bottom at the current snapshot
      DerivedProperties::SurfacePropertyPtr m_depthBasement;       ///< The depth of the basement at the current snapshot
      DerivedProperties::SurfacePropertyPtr m_depthWaterBottom;    ///< The depth of the water bottom at the current snapshot
      /// @}
   
      /// @defgroup Variables
      /// @{
      double m_sedimentDensity;       ///< The density of the entire stack of sediments
      double m_sedimentThickness;     ///< The thickness of the entire stack of sediments
      double m_backstrip;             ///< The backstrip (https://en.wikipedia.org/wiki/Back-stripping)
      double m_compensation;          ///< The compensation
      /// @}
   
      /// @defgroup ConfigFileData
      ///    Set from configuration file
      /// @{
      const double  m_mantleDensity; ///< The mantle density (is currently the same for lithospheric and asthenospheric mantle)
      const double  m_waterDensity;  ///< The water density
      double  m_densityTerm;   ///< densityTerm = 1.0 / (mantleDensity - m_waterDensity)
      double  m_airCorrection; ///< The air density correction for the TTS equation when the water depth is above the surface
      /// @}
};
#endif

