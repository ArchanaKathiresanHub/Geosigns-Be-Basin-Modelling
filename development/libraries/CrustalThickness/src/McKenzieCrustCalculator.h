//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CRUSTALTHICKNESS_MCKENZIECRUSTCALCULATOR_H_
#define _CRUSTALTHICKNESS_MCKENZIECRUSTCALCULATOR_H_

// utilitites library
#include "FormattingException.h"

//CrustalThickness
#include "AbstractInterfaceOutput.h"
#include "AbstractValidator.h"
#include "ConfigFileParameterCtc.h"
#include "InterfaceInput.h"
#include "LinearFunction.h"

// DerivedProperties library
#include "SurfaceProperty.h"

using namespace DataAccess;
using Interface::GridMap;

/// @class McKenzieCrustCalculator The calculator used to compute the thinning factor, crustal thicknesses
///    and other crustal properties from the McKenzie equations
class McKenzieCrustCalculator {

   typedef formattingexception::GeneralException McKenzieException;

   public:
   
      /// @param[in] previousThinningFactor Can be nullptr if first rifting event
      /// @param[in] previousContinentalCrustThickness Can be nullptr if first rifting event
      /// @param[in] previousOceanicCrustThickness Can be nullptr if first rifting event
      /// @throw std::invalid_argument If the inputData provided has no basement depth defined (nullptr)
      ///    or invalid constants
      McKenzieCrustCalculator( const InterfaceInput&    inputData,
                               AbstractInterfaceOutput& outputData,
                               const AbstractValidator& validator,
                               const Interface::GridMap* previousThinningFactor,
                               const Interface::GridMap* previousContinentalCrustThickness,
                               const Interface::GridMap* previousOceanicCrustThickness );
      
      ~McKenzieCrustCalculator() {};

      /// @brief Define the linear function to invert from TTS (total tectonic subsidence) to TF (thinning factor) for the (i,j) node
      /// @details Computes the function coefficients according to the function input end members
      /// @param[out] theFunction The fully defined linear function
      void defineLinearFunction(
         LinearFunction & theFunction,
         const double maxBasalticCrustThickness,
         const double magmaticDensity,
         const double thinningFactorOnsetLinearized,
         const double TTSOnsetLinearized,
         const double TTScritical
         ) const;

      /// @defgroup CrustPropertiesCalculators
      /// @{
      /// @return The estimated continental crust density
      double calculateContinentalCrustDensity( const double initialContinentalCrustThickness ) const;
      /// @return The asthenosphere potential temperature
      double calculateAstenospherePotentialTemperature( const double maxOceanicCrustThickness ) const;
      /// @return The magmatic density
      double calculateMagmaDensity( const double maxOceanicCrustThickness ) const;
      /// @}

      /// @defgroup ThinningFactorsCalculators
      /// These methods calculate the thinning factor end members for the McKenzie function f(TTS)=TF
      //  Where TTS is the total tectonic subsidence and TF is the thinning factor
      /// @{
      /// @return The thinning factor at melt onset (melting point)
      /// @details The melting point represents the depth at which the oceanic crust (basalt) starts to be created
      double calculateThinningFactorOnset( const double potentialTempAstheno, const double initialLithosphericThickness ) const;
      /// @return The linearized thinning factor at melt onset
      /// @details The McKenzie technique requires to use a linear relationship between the thinning factor (TF) and the total tectonic subsidence (TTS)
      ///    This is beacuse McKenzie the function f(TTS)=TF needs to be bijective
      double calculateThinningFactorOnsetLinearized( const double thinningFactorOnset ) const;
      /// @return The thinning factor corresponding to the maximum basalt thickness
      double calculateThinningFactorOnsetAtMaxBasalt( const double maxOceanicCrustThickness, const double thinningFactorOnset ) const;
      /// @}

      /// @defgroup TTSCalculators
      /// These methods calculate the total tectonic subsidence end members for the McKenzie function f(TTS)=TF
      /// @{
      /// @param[in] averageRiftTime The medium age of the rift event (start+end)/2
      /// @return The total tectonic subsidence at exhume point
      /// @details For the exhumed unaltered mantle
      double calculateTTSexhume( const double averageRiftTime ) const;
      /// @return The maximum total tectonic subsidence
      double calculateTTScritical( const double TTSexhume, const double maxOceanicCrustThickness, const double magmaticDensity ) const;
      /// @return The linearized total tectonic subsidence at melt onset
      /// @details The McKenzie technique requires to use a linear relationship between the thinning factor (TF) and the total tectonic subsidence (TTS)
      ///          This is beacuse McKenzie the function f(TTS)=TF needs to be bijective
      double calculateTTSOnsetLinearized( const double averageRiftTime, const double thinningFactorOnsetLinearized) const;
      /// @return The total tectonic subsidence at exhume point after serpentinization of the mantle
      /// @details This corresponds to a maximum basalt thickness of 2Km, so the subsidence correction can be approximated to 681.6394
      double calculateTTSexhumeSerpentinized( const double TTSexhume ) const;
      /// @return The (thinning factor) corrected incremental tectonic subsidence
      /// @param[in] ITS The incremental tectonic subsidence
      /// @param[in] previousTF The previous thinning factor (0 if does not exists)
      double calculateITScorrected( const double ITS, const double previousTF ) const;
      /// @}

      /// @defgroup CrustCalculators
      /// @{
      /// @return The continental crustal thickness at melt onset (melting point)
      /// @details The melting point represents the depth at which the oceanic crust (basalt) starts to be created
      double calculateContinentalCrustThicknessOnset( const double initialContinentalCrustThickness, const double TFOnsetLinearized ) const;
      /// @return The residual depth anomaly between the seelevel adjusted total tectonic subsidence and the maximum total tectonic subsidence
      double calculateResidualDepthAnomaly( const double TTScritial, const double TTSadjusted ) const;
      /// @return The continental crustal thickness
      double calculateContinentalCrustalThickness( const double thinningFactor, const double previousContinentalCrustThickness ) const;
      /// @return The oceanic (basalt) crusltal thickness
      /// @param[in] ITSadjusted The adjusted (by previous thining factor) incremental tectonic subsidence
      double calculateOceanicCrustalThickness(
         const double ITSadjusted,
         const double TTSexhume,
         const LinearFunction& linearFunction,
         const double previousOceanicCrustThickness ) const;
      /// @return The depth of the top of the oceanic (basalt) crust
      double calculateTopOceanicCrust( const double continentalCrusltalThickness, const double depthBasement ) const;
      /// @return The depth of the Moho
      /// @details The Moho is the boundary between the crust (either continental or oceanic) and the mantle
      double calculateMoho( const double topOceanicCrust, const double oceanicCrustalThickness ) const;
      /// @return The effective crustal thickness
      /// @details Represents the crust thickness as if there was no basalt
      double calculateEffectiveCrustalThickness(
         const double continentalCrusltalThickness,
         const double oceanicCrustalThickness,
         const double initialContinentalCrustThickness,
         const double initialLithosphericMantleThickness ) const;
      /// @}

      /*! @brief Computes the following map properties:
       *     - continental crust density
       *     - magmatic density
       *     - potential temperature of the asthenosphere
       *     - thinning factor at melt onset
       *     - thinning factor at melt onset linearized
       *     - thinning factor at melt onset for the maximum basalt (oceanic crust) thickness
       *     - total tectonic subsidence at exhumation point
       *     - total tectonic subsidence at critical point
       *     - total tectonic subsidence at melting point linearized
       *     - total tectonic subsidence at exhumation point serpentinized
       *     - continental crustal thickness at melt onset
       *     - residual depth anomaly
       *     - thinning factor
       *     - continental crust thickness
       *     - oceanic crust thickness
       *     - top of the oceanic crust
       *     - moho
       *     - effective crustal thickness
       */
      void compute();

   private:

      /// @brief Check the input map values for a defined node (i,j)
      /// @throw std::invalid_argument Throw exception when the input value for the node is forbiden
      void checkInputValues( const unsigned int i, const unsigned int j ) const;

      /// @defgroup DataUtilities
      /// @{
      /// @brief Retrieve input maps data
      void retrieveData();
      /// @brief Restore input maps data
      void restoreData();
      /// @brief Get the previous thinning factor value from its map (m_previousThinningFactor)
      /// @return valued(i,j) if the map is not a nullptr, 0 if the map is a nullptr
      double getPreviousTF( const unsigned int i, const unsigned int j ) const;
      /// @brief Get the previous thinning factor value from its map (m_previousContinentalCrustThickness)
      /// @return valued(i,j) if the map is not a nullptr, the initial continental crust thickness if the map is a nullptr
      double getPreviousContinentalCrustThickness( const unsigned int i, const unsigned int j ) const;
      /// @brief Get the previous thinning factor value from its map (m_previousOceanicCrustThickness)
      /// @return valued(i,j) if the map is not a nullptr, 0 if the map is a nullptr
      double getPreviousOceanicCrustThickness( const unsigned int i, const unsigned int j ) const;
      /// @}

      const CrustalThickness::ConfigFileParameterCtc& m_constants;  ///< Constants loaded from the configuration file via InterfaceInput
      const DerivedProperties::SurfacePropertyPtr m_depthBasement;  ///< The depth of the basement at the current snapshot loaded from InterfaceInput

      /// @defgroup InputMaps
      /// Loaded from InterfaceInput
      /// @{
      const GridMap& m_T0Map;        ///< Beginning of rifting event                 [Ma]
      const GridMap& m_TRMap;        ///< End of rifting event                       [Ma]
      const GridMap& m_HCuMap;       ///< Initial continental crust thickness        [m]
      const GridMap& m_HBuMap;       ///< Maximum oceanic (basaltic) crust thickness [m]
      const GridMap& m_HLMuMap;      ///< Initial lithospheric mantle thickness      [m]
      /// @}

      /// @defgroup InputMapsRange
      /// Loaded from InterfaceInput
      /// @{
      const unsigned int m_firstI; ///< First i index on the map
      const unsigned int m_firstJ; ///< First j index on the map
      const unsigned int m_lastI;  ///< Last i index on the map
      const unsigned int m_lastJ;  ///< Last j index on the map
      /// @}

      /// @defgroup previousSnapshotValues
      /// @{
      const Interface::GridMap* m_previousThinningFactor;            ///< The thinning factor of the previous computation age
      const Interface::GridMap* m_previousContinentalCrustThickness; ///< The continental crustal thickness of the previous computation age
      const Interface::GridMap* m_previousOceanicCrustThickness;     ///< The oceanic crustal thickness of the previous computation age
      /// @}

      AbstractInterfaceOutput& m_outputData;  ///< The global interface output object (contains the output maps)
      const AbstractValidator& m_validator;   ///< The validator to check if a node (i,j) is valid or not

};
#endif

