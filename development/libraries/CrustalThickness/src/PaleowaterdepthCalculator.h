//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef CRUSTALTHICKNESS_PWDCALCULATOR_H
#define CRUSTALTHICKNESS_PWDCALCULATOR_H

// CrusltalThickness library
#include "AbstractInterfaceOutput.h"
#include "AbstractValidator.h"
#include "InterfaceInput.h"

// DerivedProperties library
#include "SurfaceProperty.h"

// utilitites library
#include "FormattingException.h"

//DataAccess library
#include "Interface/GridMap.h"

using namespace DataAccess;

/// @class PaleowaterdepthCalculator The PWD calculator
class PaleowaterdepthCalculator {

   typedef formattingexception::GeneralException PWDException;

   public:
   
      /// @brief Constructs the PWD calculator in order to compute the paleowaterdepth
      /// @details Retrieve presentDayPressureTTS and currentPressureTTS data if they are not nullptr
      /// @param presentDayPressureMantle The present day pressure of the top mantle
      /// @param currentPressureMantle The current snapshot pressure of the top mantle
      /// @param presentDayPressureTTS The present day pressure at the present day TTS depth
      /// @param currentPressureTTS The current snapshot pressure at the current snapshot TTS depth
      PaleowaterdepthCalculator( const InterfaceInput&     inputData,
                                 AbstractInterfaceOutput&  outputData,
                                 const AbstractValidator&  validator,
                                 const Interface::GridMap* presentDayTTS,
                                 const Interface::GridMap* presentDayPressureTTS = 0,
                                 const Interface::GridMap* currentPressureTTS    = 0 );

      /// @details Restore presentDayPressureTTS and currentPressureTTS data if they are not nullptr
      ~PaleowaterdepthCalculator();

      /// @brief Computes the paleowaterdepth map
      void compute();

      /// @return The paleowaterdepth with the thermal correction
      /// @details The thermal correction aims to equilibrate the pressure between the TTS and the Top Mantle
      double calculatePWD( const double presentDayTTS,
                           const double backstrip,
                           const double presentDayPressureBotMantle,
                           const double presentDayPressureTTS,
                           const double currentPressureBotMantle,
                           const double currentPressureTTS ) const;

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

      const double  m_mantleDensity; ///< The mantle density (is currently the same for lithospheric and asthenospheric mantle)
      const double  m_waterDensity;  ///< The water density

      const DerivedProperties::SurfacePropertyPtr m_presentDayPressureMantle; ///< The present day pressure of the top mantle
      const DerivedProperties::SurfacePropertyPtr m_currentPressureMantle;    ///< The current snapshot pressure of the top mantle
      const Interface::GridMap* m_presentDayTTS;                              ///< The present day total tectonic subsidence (at 0.0Ma)
      const Interface::GridMap* m_presentDayPressureTTS;                      ///< The present day pressure at the present day TTS depth
      const Interface::GridMap* m_currentPressureTTS;                         ///< The current snapshot pressure at the current snapshot TTS depth

      AbstractInterfaceOutput& m_outputData; ///< The global interface output object (contains the output maps)
      const AbstractValidator& m_validator;  ///< The validator to check if a node (i,j) is valid or not
};
#endif

