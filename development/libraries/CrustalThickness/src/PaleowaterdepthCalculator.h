//                                                                      
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef CRUSTALTHICKNESS_PWDCALCULATOR_H
#define CRUSTALTHICKNESS_PWDCALCULATOR_H

// DerivedProperties library
#include "SurfaceProperty.h"

//Forward declare
class AbstractInterfaceOutput;
class AbstractValidator;
class InterfaceInput;

namespace DataAccess {
   namespace Interface {
      class GridMap;
   }
}

using namespace DataAccess;

namespace CrustalThickness
{
   /// @class PaleowaterdepthCalculator The PWD calculator
   class PaleowaterdepthCalculator {

   public:

      /// @brief Constructs the PWD calculator in order to compute the paleowaterdepth
      /// @details Retrieve presentDayPressureBasement and currentPressureBasement data if they are not nullptr
      /// @param[in] presentDayTTS The present day Total Tectonic Subsidence
      PaleowaterdepthCalculator( const InterfaceInput&     inputData,
         AbstractInterfaceOutput&  outputData,
         const AbstractValidator&  validator,
         const Interface::GridMap* presentDayTTS );

      /// @details Restore presentDayPressureBasement and currentPressureBasement data if they are not nullptr
      ~PaleowaterdepthCalculator();

      /// @brief Computes the paleowaterdepth map
      void compute();

      /// @return The response factor used during the thermally corrected paleowaterdepth computation
      double calculateResponseFactor( const double presentDayPressureBotMantle,
         const double presentDayPressureBasement,
         const double currentPressureBotMantle,
         const double currentPressureBasement ) const;

      /// @return The paleowaterdepth with the thermal correction
      /// @details The thermal correction aims to equilibrate the pressure between the Basement and the bottom Mantle
      double calculateThermallyCorrectedPWD( const double presentDayTTS,
         const double backstrip,
         const double responseFactor ) const;

      /// @return The paleowaterdepth without the thermal correction
      double calculatePWD( const double presentDayTTS,
         const double backstrip ) const;

   private:

      /// @defgroup DataUtilities
      /// @{
      /// @brief Retrieve pressure maps data
      void retrieveData();
      /// @brief Restore pressure maps data
      void restoreData();
      /// @}

      const unsigned int m_firstI; ///< First i index on the map
      const unsigned int m_firstJ; ///< First j index on the map
      const unsigned int m_lastI;  ///< Last i index on the map
      const unsigned int m_lastJ;  ///< Last j index on the map

      const double  m_mantleDensity; ///< The mantle density (is currently the same for lithospheric and asthenospheric mantle) [kg.m-3]
      const double  m_waterDensity;  ///< The water density                                                                     [kg.m-3]

      const DerivedProperties::SurfacePropertyPtr m_presentDayPressureMantle;   ///< The present day pressure of the top mantle                           [MPa]
      const DerivedProperties::SurfacePropertyPtr m_currentPressureMantle;      ///< The current snapshot pressure of the top mantle                      [MPa]
      const DerivedProperties::SurfacePropertyPtr m_presentDayPressureBasement; ///< The present day pressure at the present day basement depth           [MPa]
      const DerivedProperties::SurfacePropertyPtr m_currentPressureBasement;    ///< The current snapshot pressure at the current snapshot basement depth [MPa]
      const Interface::GridMap* m_presentDayTTS;                                ///< The present day total tectonic subsidence                            [m]

      AbstractInterfaceOutput& m_outputData; ///< The global interface output object (contains the output maps)
      const AbstractValidator& m_validator;  ///< The validator to check if a node (i,j) is valid or not
   };
} // End namespace CrustalThickness
#endif

